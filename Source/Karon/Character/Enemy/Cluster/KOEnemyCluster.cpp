// Fill out your copyright notice in the Description page of Project Settings.


#include "KOEnemyCluster.h"

#include "NavigationSystem.h"
#include "Character/Enemy/KOBaseEnemy.h"
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
	
	
	for (auto EnemyPair : EnemyMap)
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
				Enemy->SetupEnemy(DataSubsystem,Level);
				Enemy->OnEnemyDead.AddDynamic(this,&ThisClass::OnDestroyedEnemy);
				SpawnedEnemiesCount++;
			}
		}
	}
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
