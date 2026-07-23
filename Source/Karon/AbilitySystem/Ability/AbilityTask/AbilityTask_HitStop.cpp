#include "AbilityTask_HitStop.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Utility/Log/KOLogManager.h"

UAbilityTask_HitStop* UAbilityTask_HitStop::HitStop(
	UGameplayAbility* OwningAbility, 
	AActor* TargetActor,
	float Duration,
	float TimeDilation,
	bool bAffectInstigator)
{
	UAbilityTask_HitStop* Task = NewAbilityTask<UAbilityTask_HitStop>(OwningAbility);
	Task->StopDuration = Duration;
	Task->StopDilation = FMath::Clamp(TimeDilation, 0.f, 1.f);
	Task->bAffectInstigatorToo = bAffectInstigator;
	
	if (ACharacter* TargetChar = Cast<ACharacter>(TargetActor))
	{
		Task->CachedInstigatorMesh = TargetChar->GetMesh();
	}
    
	if (ACharacter* AttackerChar = Cast<ACharacter>(OwningAbility->GetAvatarActorFromActorInfo()))
	{
		Task->CachedAvatarMesh = AttackerChar->GetMesh();
	}
	return Task;
}

void UAbilityTask_HitStop::Activate()
{
	SetMeshAnimRate(CachedInstigatorMesh.Get(), StopDilation);
	if (bAffectInstigatorToo)
	{
		SetMeshAnimRate(CachedAvatarMesh.Get(), StopDilation);	
	}

	// 복구 타이머 
	GetWorld()->GetTimerManager().SetTimer(
		RecoveryTimer, this, &UAbilityTask_HitStop::RecoverTime, StopDuration, false);
	
	KO_LOG(GAS, Warning, TEXT("HitStop Task : Timer Start."));
}

void UAbilityTask_HitStop::OnDestroy(bool bInOwnerFinished)
{
	KO_LOG(GAS, Warning, TEXT("HitStop Task : OnDestroy."));
	
	GetWorld()->GetTimerManager().ClearTimer(RecoveryTimer);
	
	
	SetMeshAnimRate(CachedInstigatorMesh.Get(), 1.f);
	if (bAffectInstigatorToo)
	{
		SetMeshAnimRate(CachedAvatarMesh.Get(), 1.f);
	}
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_HitStop::SetMeshAnimRate(USkeletalMeshComponent* Mesh, float Rate)
{
	if (!Mesh) return;
	
	Mesh->GlobalAnimRateScale = Rate;
	
	if (UAnimInstance* AnimInst = Mesh->GetAnimInstance())
	{
		if (AnimInst->IsAnyMontagePlaying())
		{
			if (UAnimMontage* CurrentMontage = AnimInst->GetCurrentActiveMontage())
			{
				AnimInst->Montage_SetPlayRate(CurrentMontage, Rate);
			}
		}
	}
}

void UAbilityTask_HitStop::RecoverTime()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnFinished.Broadcast();	
	}
	
	EndTask();
	
	KO_LOG(GAS, Warning, TEXT("HitStop Task : Timer End."));
}
