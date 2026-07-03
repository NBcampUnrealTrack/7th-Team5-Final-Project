// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetourCrowdAIController.h"
#include "KOBaseEnemyAIController.generated.h"

class AKOBaseEnemy;


UCLASS()
class KARON_API AKOBaseEnemyAIController : public ADetourCrowdAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AKOBaseEnemyAIController();
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	FORCEINLINE virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }

protected:
	virtual void OnPossess(APawn* InPawn) override;

private:
	
	UFUNCTION()
	void HitEvent(bool bIsHit);
	
	UFUNCTION()
	void DeadEvent();
	
	UFUNCTION()
	void ResetEvent();
	
	UFUNCTION()
	void SetAI(
		UBehaviorTree* ParamBT,
		float AttackRadius,
		bool  bIsLongRange,
		float Speed,
		float StrafeSpeed,
		float EnemyAttackDelay
		);
	
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	void MakeAIPerceptionTeamEvent(AActor* TargetActor);
	void StopBT();
	void SetTargetActor(AActor* TargetActor);
	
	
protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBehaviorTree> EnemyBehaviorTree;
	
	UPROPERTY(EditAnywhere)
	UBlackboardComponent* BBComp;
	
	//bool BB 키(FName)
	const FName bIsDeadKey=TEXT("bIsDead");
	const FName bIsHitKey = TEXT("bIsHit");
	const FName bIsLongRangeKey = TEXT("bIsLongRange");
	const FName SelfActorKey = TEXT("SelfActor");
	const FName AttackRadiusKey = TEXT("AttackRadius");
	const FName SpeedKey = TEXT("Speed");
	const FName StrafeSpeedKey = TEXT("StrafeSpeed");
	const FName EnemyAttackDelayTimeKey = TEXT("EnemyAttackDelayTime");
	const FName bCanAttackKey = TEXT("bCanAttack");
	const FName bCanPatrolKey = TEXT("bCanPatrol");
	const FName InitialLocationKey = TEXT("InitialLocation");
	const FName MaxDistanceKey = TEXT("MaxDistance");
	
	
	UPROPERTY()
	TObjectPtr<AKOBaseEnemy> Enemy;
	
	//AI퍼셉션
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;
	//팀 이벤트 트리거 적용 범위
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float TeamEventDistance=1500.f;
	
	//HP가 0이 된 이후 해당 딜레이 이후에 BT를 멈춥니다.
	float StopBTDelay=1.f;
	
	UPROPERTY(EditDefaultsOnly,Category="Attribute")
	float MaxDistanceFromInit= 3000.f; 
	
private:
	FGenericTeamId TeamId;
	FTimerHandle TimerHandle;
	bool bIsDead=false;
	
	FVector InitialLocation=FVector::ZeroVector;
	

};
