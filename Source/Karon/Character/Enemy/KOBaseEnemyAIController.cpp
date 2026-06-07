// Fill out your copyright notice in the Description page of Project Settings.


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


// Sets default values
AKOBaseEnemyAIController::AKOBaseEnemyAIController()
{
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
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
	Enemy->OnCharacterHit.BindUObject(this, &AKOBaseEnemyAIController::HitEvent);
	Enemy->OnEnemyDead.AddDynamic(this, &AKOBaseEnemyAIController::DeadEvent);
	Enemy->OnCharacterReset.BindUObject(this, &AKOBaseEnemyAIController::ResetEvent);
	//TODO: SetAI 구현
	//Enemy->OnCharacterSetAI.AddDynamic(this,&AKOBaseEnemyAIController::SetAI);
	Enemy->OnGameplayAbilityEnd.BindUObject(this,&AKOBaseEnemyAIController::OnGameplayAbilityEnd);
	AIPerceptionComp->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &AKOBaseEnemyAIController::OnTargetPerceptionUpdated);

	//TeamId 설정
	TeamId = FGenericTeamId(1);
	//임시 설정. 풀 관리시 사용
	SetAI(EnemyBehaviorTree,EnemyAttackRadius,bIsEnemyLongRange,EnemySpeed,EnemyStrafeSpeed,EnemyAttackDelayTime);
}

ETeamAttitude::Type AKOBaseEnemyAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	const APawn* OtherPawn = Cast<APawn>(&Other);
	const ACharacter* OtherCharacter = Cast<ACharacter>(&Other);
	if (OtherCharacter==nullptr)
	{
		return ETeamAttitude::Neutral;
	}
	if (const AKOHeroCharacter* Player=Cast<AKOHeroCharacter>(&Other))
	{
		return ETeamAttitude::Hostile;
	}
	else if (const AKOBaseEnemy* OtherEnemy=Cast<AKOBaseEnemy>(&Other))
	{
		return ETeamAttitude::Friendly;
	}
	return ETeamAttitude::Neutral;
}

void AKOBaseEnemyAIController::OnGameplayAbilityEnd()
{
	if (BBComp!=nullptr)
	{
		BBComp->SetValueAsBool(bIsMontageEndKey, true);
	}
}

void AKOBaseEnemyAIController::HitEvent()
{
	if (BBComp!=nullptr)
	{
		BBComp->SetValueAsBool(bIsHitKey, true);
	}
}

void AKOBaseEnemyAIController::DeadEvent()
{
	if (BBComp!=nullptr&&!bIsDead)
	{
		bIsDead=true;
		BBComp->SetValueAsBool(bIsDeadKey, true);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle,this,&AKOBaseEnemyAIController::StopBT,StopBTDelay,false);
	}
}

void AKOBaseEnemyAIController::ResetEvent()
{
	if (BBComp!=nullptr)
	{
		BBComp->InitializeBlackboard(*EnemyBehaviorTree->BlackboardAsset);
		//명시적 초기화
		BBComp->SetValueAsBool(bIsDeadKey,false);
		BBComp->SetValueAsBool(bIsHitKey,false);
		RunBehaviorTree(EnemyBehaviorTree);
	}
}

void AKOBaseEnemyAIController::SetAI(UBehaviorTree* ParamBT, float AttackRadius, bool bIsLongRange, float Speed,
	float StrafeSpeed, float EnemyAttackDelay)
{
	//TODO: 비동기 로드시 AIController세팅
	
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
		
		if (Enemy)
		{
			BBComp->SetValueAsBool(bCanAttackKey,Enemy->bCanAttack);
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
				BBComp->SetValueAsObject(TEXT("TargetActor"), Player);
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
	if (TargetActor!=nullptr)
	{
		BBComp->SetValueAsObject(TEXT("TargetActor"), TargetActor);
		if (IsValid(Enemy))
		{
			Enemy->OnBattleChanged(true);
		}
	}
	else
	{
		BBComp->ClearValue(TEXT("TargetActor"));
		if (IsValid(Enemy))
		{
			Enemy->OnBattleChanged(false);
		}
	}
}


