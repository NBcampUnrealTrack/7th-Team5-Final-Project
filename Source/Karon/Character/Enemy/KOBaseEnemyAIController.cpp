#include "KOBaseEnemyAIController.h"

#include "KOBaseEnemy.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Hero/KOHeroCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Prediction.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Team.h"
#include "Subsystem/KOSaveSubsystem.h"


AKOBaseEnemyAIController::AKOBaseEnemyAIController()
{
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
}

void AKOBaseEnemyAIController::ResetPlayerDetection()
{
	SetTargetActor(nullptr);
	ClearFocus(EAIFocusPriority::Gameplay);

	if (AIPerceptionComp)
	{
		AIPerceptionComp->ForgetAll();
	}

	if (BBComp)
	{
		BBComp->ClearValue(TEXT("TargetActor"));
		BBComp->ClearValue(TEXT("DetectLocation"));
	}
}

void AKOBaseEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	Enemy = Cast<AKOBaseEnemy>(InPawn);
	if (!IsValid(Enemy))
	{
		return;
	}
	
	//Bindings
	Enemy->OnHitEvent.BindUObject(this, &AKOBaseEnemyAIController::HitEvent);
	Enemy->OnCounterAttackEvent.BindUObject(this, &AKOBaseEnemyAIController::CounterAttackEvent);
	Enemy->OnCanAttackEvent.BindUObject(this, &AKOBaseEnemyAIController::CanAttackEvent);
	Enemy->OnEnemyDead.AddDynamic(this, &AKOBaseEnemyAIController::DeadEvent);
	Enemy->OnCharacterReset.BindUObject(this, &AKOBaseEnemyAIController::ResetEvent);
	Enemy->OnHalfHealthEvent.BindUObject(this,&AKOBaseEnemyAIController::LevelUpEvent);
	
	AIPerceptionComp->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &AKOBaseEnemyAIController::OnTargetPerceptionUpdated);

	//TeamId 설정
	TeamId = FGenericTeamId(1);
	
	InitialLocation=InPawn->GetActorLocation();

	
	//임시 설정. 풀 관리시 사용
	SetAI(EnemyBehaviorTree,
		Enemy->EnemyAttackRadius,
		Enemy->bIsEnemyLongRange,
		Enemy->EnemySpeed,
		Enemy->EnemyStrafeSpeed,
		Enemy->EnemyAttackDelayTime);
	
}



ETeamAttitude::Type AKOBaseEnemyAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (const AKOHeroCharacter* Player = Cast<AKOHeroCharacter>(&Other))
	{
		return ETeamAttitude::Hostile;
	}
	else if (const AKOBaseEnemy* OtherEnemy = Cast<AKOBaseEnemy>(&Other))
	{
		return ETeamAttitude::Friendly;
	}
	
	return ETeamAttitude::Neutral;
}

void AKOBaseEnemyAIController::HitEvent(bool bIsHit)
{
	if (BBComp != nullptr)
	{
		BBComp->SetValueAsBool(bIsHitKey, bIsHit);
	}
}

void AKOBaseEnemyAIController::CounterAttackEvent(bool bIsTriggered)
{
	if (BBComp != nullptr)
	{
		BBComp->SetValueAsBool(bIsCounterAttackKey,bIsTriggered);
	}
}

void AKOBaseEnemyAIController::CanAttackEvent(bool bIsTriggered)
{
	if (BBComp != nullptr)
	{
		BBComp->SetValueAsBool(bCanAttackKey,bIsTriggered);
		Enemy->bCanAttack=bIsTriggered;
	}
}

void AKOBaseEnemyAIController::DeadEvent()
{
	if (BBComp != nullptr && !bIsDead)
	{
		bIsDead = true;
		SetTargetActor(nullptr);
		
		if (UKOSaveSubsystem* SaveSubsystem = UKOSaveSubsystem::Get(this))
		{
			SaveSubsystem->NotifyActorStoppedTargetingPlayer(IsValid(Enemy) ? Enemy : GetPawn());
		}
		
		BBComp->SetValueAsBool(bIsDeadKey, true);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle,this,&AKOBaseEnemyAIController::StopBT,StopBTDelay,false);
	}
}

void AKOBaseEnemyAIController::ResetEvent()
{
	if (BBComp!=nullptr)
	{
		SetTargetActor(nullptr);
		BBComp->InitializeBlackboard(*EnemyBehaviorTree->BlackboardAsset);
		//명시적 초기화
		BBComp->SetValueAsBool(bIsDeadKey,false);
		BBComp->SetValueAsBool(bIsHitKey,false);
		bIsDead = false;
		RunBehaviorTree(EnemyBehaviorTree);
	}
}

void AKOBaseEnemyAIController::LevelUpEvent(bool bIsTriggered)
{
	if (bIsTriggered)
	{
		BBComp->SetValueAsInt(LevelKey,BBComp->GetValueAsInt(LevelKey)+1);
	}
}

void AKOBaseEnemyAIController::SetAI(
	UBehaviorTree* ParamBT, float AttackRadius, bool bIsLongRange, float Speed,
	float StrafeSpeed, float EnemyAttackDelay)
{

	EnemyBehaviorTree = ParamBT;
	
	if (UseBlackboard(EnemyBehaviorTree->GetBlackboardAsset(), BBComp))
	{
		if (BBComp==nullptr)
		{
			return;
		}
		BBComp->SetValueAsObject(SelfActorKey, GetPawn());
		BBComp->SetValueAsFloat(AttackRadiusKey, AttackRadius);
		BBComp->SetValueAsBool(bIsLongRangeKey, bIsLongRange);
		BBComp->SetValueAsFloat(SpeedKey, Speed);
		BBComp->SetValueAsFloat(StrafeSpeedKey, StrafeSpeed);
		BBComp->SetValueAsFloat(EnemyAttackDelayTimeKey, EnemyAttackDelay);
		BBComp->SetValueAsVector(InitialLocationKey,InitialLocation);
		BBComp->SetValueAsFloat(MaxDistanceKey, MaxDistanceFromInit);
		
		// 초기 시작 레벨은 1
		BBComp->SetValueAsInt(LevelKey,1);
		
		if (Enemy)
		{
			BBComp->SetValueAsBool(bCanAttackKey,Enemy->bCanAttack);
			BBComp->SetValueAsBool(bCanPatrolKey,Enemy->bCanPatrol);
		}
		
		RunBehaviorTree(EnemyBehaviorTree);
	}
	
}

void AKOBaseEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (BBComp == nullptr)
	{
		return;
	}
	//피격당했을 경우
	if (Stimulus.Type==UAISense::GetSenseID<UAISense_Damage>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			if (AKOHeroCharacter* Player=Cast<AKOHeroCharacter>(Actor))
			{
				SetTargetActor(Player);
				//팀에게도 전달
				MakeAIPerceptionTeamEvent(Player);
			}
		}
		return;
	}
	
	//시각으로 본 경우
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			if (AKOHeroCharacter* Player=Cast<AKOHeroCharacter>(Actor))
			{
				SetTargetActor(Player);
			}
		}
		else
		{
			//Prediction 자극을 0.5초 뒤 예상 위치에 남김.
			UAISense_Prediction::RequestPawnPredictionEvent(GetPawn(),Actor,0.25f);
			SetTargetActor(nullptr);
		}
		
		return;
	}
	//청각으로 들은 경우.
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			GetBlackboardComponent()->SetValueAsVector(TEXT("DetectLocation"), Stimulus.StimulusLocation);
		}
		return;
	}
	//예측 마커가 찍힌 경우
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Prediction>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			GetBlackboardComponent()->SetValueAsVector(TEXT("DetectLocation"), Stimulus.StimulusLocation);
		}
		
		return;
	}
	//팀이 피격당한경우
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Team>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			SetTargetActor(Actor);
		}
		
		return;
	}
}

void AKOBaseEnemyAIController::MakeAIPerceptionTeamEvent(AActor* TargetActor)
{
	FAITeamStimulusEvent TeamEvent(
	GetPawn(),   
	TargetActor, 
	GetPawn()->GetActorLocation(),   
	TeamEventDistance
	);
	
	if (UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(GetWorld()))
	{
		PerceptionSystem->OnEvent(TeamEvent);
	}
}

void AKOBaseEnemyAIController::StopBT()
{
	UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(GetBrainComponent());
	if (BTComp)
	{
		BTComp->StopTree(EBTStopMode::Safe);
	}
}

void AKOBaseEnemyAIController::SetTargetActor(AActor* TargetActor)
{
	if (!BBComp)
	{
		return;
	}

	AActor* PreviousTarget = Cast<AActor>(BBComp->GetValueAsObject(TEXT("TargetActor")));

	const bool bWasTargetingPlayer = Cast<AKOHeroCharacter>(PreviousTarget) != nullptr;
	const bool bIsTargetingPlayer = Cast<AKOHeroCharacter>(TargetActor) != nullptr;
	
	if (TargetActor!=nullptr)
	{
		BBComp->SetValueAsObject(TEXT("TargetActor"), TargetActor);
		if (IsValid(Enemy))
		{
			Enemy->TargetActor=TargetActor;
			Enemy->OnBattleChanged(true);
		}
	}
	else
	{
		BBComp->ClearValue(TEXT("TargetActor"));
		if (IsValid(Enemy))
		{
			Enemy->TargetActor=nullptr;
			Enemy->OnBattleChanged(false);
		}
	}
	
	if (UKOSaveSubsystem* SaveSubsystem = UKOSaveSubsystem::Get(this))
	{
		if (!bWasTargetingPlayer && bIsTargetingPlayer)
		{
			SaveSubsystem->NotifyActorTargetingPlayer(GetPawn());
		}
		else if (bWasTargetingPlayer && !bIsTargetingPlayer)
		{
			SaveSubsystem->NotifyActorStoppedTargetingPlayer(GetPawn());
		}
	}
}




