#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_HitStop.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHitStopFinishedDelegate);

UCLASS()
class KARON_API UAbilityTask_HitStop : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta=(HidePin="OwningAbility", DefaultToSelf="OwningAbility"))
	static UAbilityTask_HitStop* HitStop(
		UGameplayAbility* OwningAbility,
		AActor* TargetActor,
		float Duration          = 0.06f,
		float TimeDilation      = 0.0f,
		bool  bAffectInstigator = true
	);
	
	virtual void Activate() override;
	
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
private:
	void SetMeshAnimRate(USkeletalMeshComponent* Mesh, float Rate);
	
	void RecoverTime();
	
public:
	UPROPERTY(BlueprintAssignable)
	FHitStopFinishedDelegate OnFinished;
	
private:
	float StopDuration;
	float StopDilation;
	bool bAffectInstigatorToo;
	
	FTimerHandle RecoveryTimer;
	
	TWeakObjectPtr<AActor> CachedAvatar;
	TWeakObjectPtr<AActor> CachedInstigator;
	
	TWeakObjectPtr<USkeletalMeshComponent> CachedAvatarMesh;
	TWeakObjectPtr<USkeletalMeshComponent> CachedInstigatorMesh;
};
