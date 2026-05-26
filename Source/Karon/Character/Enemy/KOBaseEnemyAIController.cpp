// Fill out your copyright notice in the Description page of Project Settings.


#include "KOBaseEnemyAIController.h"

#include "KOBaseEnemy.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
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
	Enemy->OnCharacterDead.BindUObject(this, &AKOBaseEnemyAIController::DeadEvent);
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
	//TODO: 태그 대신 Cast로 조건 변경
	if (Other.ActorHasTag(TEXT("Player")))
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
	if (BBComp!=nullptr)
	{
		BBComp->SetValueAsBool(bIsDeadKey, true);
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
			if (Actor->ActorHasTag(TEXT("Player")))
			{
				BBComp->SetValueAsObject(TEXT("TargetActor"), Actor);
				//팀에게도 전달
				MakeAIPerceptionTeamEvent(Actor);
			}
		}
		return;
	}
	
	//시각으로 본 경우
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			if (Actor->ActorHasTag(TEXT("Player")))
			{
				BBComp->SetValueAsObject(TEXT("TargetActor"), Actor);
			}
		}
		else
		{
			//Prediction 자극을 0.5초 뒤 예상 위치에 남김.
			UAISense_Prediction::RequestPawnPredictionEvent(GetPawn(),Actor,0.25f);
			BBComp->ClearValue(TEXT("TargetActor"));
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
			BBComp->SetValueAsObject(TEXT("TargetActor"), Actor);
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


