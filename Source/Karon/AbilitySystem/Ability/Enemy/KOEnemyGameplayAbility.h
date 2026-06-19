#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/Attack/KOGA_AttackBase.h"
#include "KOEnemyGameplayAbility.generated.h"

UCLASS()
class KARON_API UKOEnemyGameplayAbility : public UKOGA_AttackBase
{
	GENERATED_BODY()
	
public:
	UKOEnemyGameplayAbility();
	
	virtual void ActivateAbility(
	   const FGameplayAbilitySpecHandle Handle,
	   const FGameplayAbilityActorInfo* ActorInfo,
	   const FGameplayAbilityActivationInfo ActivationInfo,
	   const FGameplayEventData* TriggerEventData
   ) override;

private:
	UFUNCTION()
	void OnMontageCompleted();
	
	UFUNCTION()
	void OnMontageCancelled();
	
	UFUNCTION()
	void OnNotifyHitEvent(FGameplayEventData HitGameplayEventData);

public:
	 // 이전 Notify틱의 소켓의 위치
	 UPROPERTY()
	 FVector PresentAttackSocketLocation = FVector::ZeroVector;
	
	 //현재 Notify틱의 소켓의 위치
	 UPROPERTY()
	 FVector CurrentAttackSocketLocation = FVector::ZeroVector;
	
	 //현재 Notify에서 플레이어를 타격하였는지 여부(중복 타격 방지, 싱글플레이)
	 UPROPERTY()
	 bool bIsAttacked=false;

};
