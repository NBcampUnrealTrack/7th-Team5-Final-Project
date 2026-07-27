// Fill out your copyright notice in the Description page of Project Settings.


#include "KOEnemyCluster.h"

#include "NavigationSystem.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "SubSystem/KOEnemyDataSubsystem.h"


// Sets default values
AKOEnemyCluster::AKOEnemyCluster()
{
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);
    
	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
	SpawningBox->SetupAttachment(Scene);
}

// Called when the game starts or when spawned
void AKOEnemyCluster::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnEnemies();
	
	GetWorld()->GetTimerManager().SetTimer(
		AttackEvalTimerHandle, this, &ThisClass::EvaluateAttackers,
		EvalInterval, true);
}


void AKOEnemyCluster::SpawnEnemies()
{
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSystem==nullptr)
	{
		return;
	}
	//DataSubsystem 캐싱
	UKOEnemyDataSubsystem* DataSubsystem=UKOEnemyDataSubsystem::Get(this);
	if (DataSubsystem==nullptr)
	{
		return;
	}
	
	const FVector BoxExtent = SpawningBox->GetScaledBoxExtent();
	const FVector BoxOrigin = SpawningBox->GetComponentLocation();
	
	//이미 스폰된 위치를 저장
	TArray<FVector> SpawnedLocations;
	
	SpawnedEnemiesCount=0;
	DestroyedEnemyCnt=0;
	ManagedEnemies.Reset();
	
	
	for (auto& EnemyPair : EnemyMap)
	{
		for (int32 i=0;i<EnemyPair.Value;i++)
		{
			//스폰가능한 표본이 있는지 여부
			bool bValidPointFound = false;
			//표본이 될 수 있는 후보 포인트
			FVector CandidatePoint = FVector::ZeroVector;
			
			//기존 표본과 일정 이상 거리를 두는 다른 표본 획득을 해당 횟수만큼 시도
			for (int32 j=0;j<MaxAttemptsPerPoint;j++)
			{
				//Volume내 포인트를 가져온다.
				FVector RandomPointInVolume = UKismetMathLibrary::RandomPointInBoundingBox(BoxOrigin, BoxExtent);
				FNavLocation NavLocation;
				//Projection 범위
				FVector QueryExtent(ProjectionDistance, ProjectionDistance, BoxExtent.Z);
				
				//NavMesh로의 투영이 가능한 경우
				if (NavSystem->ProjectPointToNavigation(RandomPointInVolume, NavLocation, QueryExtent))
				{
					CandidatePoint = NavLocation.Location;
					bool bTooClose = false;

					//거리 검사
					for (auto& ExistingLoc : SpawnedLocations)
					{
						//거리가 가까운 경우 탈락
						if (FVector::DistSquaredXY(CandidatePoint, ExistingLoc) < FMath::Square(MinDistanceBetweenEnemies))
						{
							bTooClose = true;
							break;
						}
					}
					//기존의 모든 표본보다 MinDistanceBetweenEnemies이상 멀 경우
					if (!bTooClose)
					{
						bValidPointFound = true;
						break;
					}
				}
			}
			//스폰가능한 표본을 찾은 경우
			if (bValidPointFound)
			{
				//기존 TArray에 추가
				SpawnedLocations.Add(CandidatePoint);
				//다른 액터들과 겹칠 경우 충돌을 피해서 반드시 스폰
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
				
				//랜덤한 방향으로 스폰
				FRotator RandomRotation(0.f, UKismetMathLibrary::RandomFloatInRange(0.f, 360.f), 0.f);
				//Z오프셋을 더해 Enemy가 바닥에 끼는 현상 방지
				CandidatePoint=CandidatePoint+FVector(0,0,EnemyZOffset);
				AKOBaseEnemy* Enemy=GetWorld()->SpawnActor<AKOBaseEnemy>(EnemyPair.Key, CandidatePoint, RandomRotation, SpawnParams);
				if (!Enemy) { continue; }
				ManagedEnemies.Add(Enemy);
				
				const FName MonsterSaveId = FName(*FString::Printf(
					TEXT("%s_Wave%d_%d"),
					*ClusterSaveId.ToString(),
					SpawnWaveIndex,
					SpawnedEnemiesCount
				));
				Enemy->SetMonsterSaveInfoForLoad(ClusterSaveId, MonsterSaveId);
				Enemy->SetupEnemy(DataSubsystem,Level);
				Enemy->OnEnemyDead.AddDynamic(this,&ThisClass::OnDestroyedEnemy);
				SpawnedEnemiesCount++;
			}
		}
	}
	SpawnWaveIndex++;
}

void AKOEnemyCluster::OnDestroyedEnemy()
{
	DestroyedEnemyCnt++;
	
	//클러스터의 에너미가 전부 죽었다면, 해당 인터벌 후 재스폰
	if (DestroyedEnemyCnt==SpawnedEnemiesCount)
	{
		GetWorld()->GetTimerManager().SetTimer(
			SpawnTimerHandle,
			this,
			&ThisClass::SpawnEnemies,
			SpawnInterval,
			false
			);
	}
}

void AKOEnemyCluster::EvaluateAttackers()
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (Player == nullptr)
	{
		return;
	}

	// 죽은 에너미 정리. RemoveAtSwap이므로 뒤에서부터 순회
	for (int32 i = ManagedEnemies.Num() - 1; i >= 0; --i)
	{
		if (!ManagedEnemies[i].Get())
		{
			ManagedEnemies.RemoveAtSwap(i);
		} 
	}

	// 타입별 별도 토큰에 따른 별도 힙(우선순위 큐) 이후 TSet에 추가
	TSet<AKOBaseEnemy*> Winners;
	Winners.Reserve(MaxShortRangeTokenNum + MaxLongRangeTokenNum);

	SelectWinnersForType(false,  MaxShortRangeTokenNum,  Player, Winners);
	SelectWinnersForType(true, MaxLongRangeTokenNum, Player, Winners);

	// 토큰에 따른 에너미 공격 여부 갱신
	for (const TWeakObjectPtr<AKOBaseEnemy>& WeakEnemy : ManagedEnemies)
	{
		AKOBaseEnemy* Enemy = WeakEnemy.Get();
		if (!Enemy)
		{
			continue;
		}
		
		//현재 토큰 소지 여부
		bool bNowWinner  = Winners.Contains(Enemy);
		
		//이전 평가에서 토큰 소지 여부
		bool bWasWinner  = PrevWinners.Contains(Enemy);
		
		// 상태가 바뀐 에너미에게만 새로 델리게이트를 실행한다.
		if (bNowWinner != bWasWinner)   
		{
			Enemy->OnCanAttackEvent.ExecuteIfBound(bNowWinner);
		}
	}
	
	//토큰을 받은 에너미 목록을 갱신
	PrevWinners.Reset();
	for (AKOBaseEnemy* Winner : Winners)
	{
		PrevWinners.Add(Winner);
	}
	
}

void AKOEnemyCluster::SelectWinnersForType(bool bIsLongRange, int32 TokenNum, APawn* Player,
	TSet<AKOBaseEnemy*>& OutWinners)
{
	if (TokenNum <= 0)
	{
		return;
	}
	// 메모리 할당은 한번만 하고 재사용
	AttackCandidates.Reset();

	// 해당 타입 후보만 수집
	for (auto& WeakEnemy : ManagedEnemies)
	{
		AKOBaseEnemy* Enemy = WeakEnemy.Get();
		
		if (!Enemy || Enemy->GetIsLongRange() != bIsLongRange)
		{
			continue;
		} 
		
		//현재 플레이어를 인식하지 못한 적도 후보에서 제외
		if (Enemy->TargetActor==nullptr)
		{
			continue;
		}
		
		//해당 역할의 에너미(WeakPtr)와 점수를 별도의 배열에 저장, 복사가 없는 Emplace 사용
		AttackCandidates.Emplace(Enemy, ComputeAttackPriority(Enemy, Player));
	}
	if (AttackCandidates.Num() == 0)
	{
		return;
	}

	// 우선순위 min-heap. 상위 K명만 필요 → 전체 정렬 O(N log N) 대신
	// Heapify O(N) + HeapPop K회 O(K log N)
	auto Pred = [](const TPair<TWeakObjectPtr<AKOBaseEnemy>, float>& A,
				   const TPair<TWeakObjectPtr<AKOBaseEnemy>, float>& B)
	{
		return A.Value < B.Value;
	};
	
	//힙으로 변환
	AttackCandidates.Heapify(Pred);

	// 토큰 수와 남은 에너미 수 중 적은 것을 사용
	const int32 K = FMath::Min(TokenNum, AttackCandidates.Num());
	
	for (int32 i = 0; i < K; ++i)
	{
		TPair<TWeakObjectPtr<AKOBaseEnemy>, float> Top;
		AttackCandidates.HeapPop(Top, Pred, EAllowShrinking::No);
		
		// 한번 더 유효성 체크
		if (AKOBaseEnemy* Enemy = Top.Key.Get())
		{
			//여기서는 타입에 가리지 않고 Add
			OutWinners.Add(Enemy);
		}
	}
}

float AKOEnemyCluster::ComputeAttackPriority(AKOBaseEnemy* Enemy, APawn* Player)
{
	return (Enemy->GetActorLocation() - Player->GetActorLocation()).SizeSquared();
}

void AKOEnemyCluster::ResetClusterForLoad()
{
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);

	SpawnedEnemiesCount = 0;
	DestroyedEnemyCnt = 0;
}

void AKOEnemyCluster::RegisterSpawnedEnemyForLoad(AKOBaseEnemy* Enemy)
{
	if (!Enemy)
	{
		return;
	}
	
	ManagedEnemies.Add(Enemy);
	Enemy->OnEnemyDead.AddUniqueDynamic(this, &ThisClass::OnDestroyedEnemy);
	SpawnedEnemiesCount++;
}

void AKOEnemyCluster::ScheduleRespawnForLoad()
{
	if (!GetWorld())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);

	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&ThisClass::SpawnEnemies,
		SpawnInterval,
		false
	);
}
