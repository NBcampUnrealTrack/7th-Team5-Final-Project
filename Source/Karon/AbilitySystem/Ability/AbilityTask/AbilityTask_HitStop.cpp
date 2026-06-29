#include "AbilityTask_HitStop.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Utility/Log/KOLogManager.h"

UAbilityTask_HitStop* UAbilityTask_HitStop::HitStop(
	UGameplayAbility* OwningAbility, 
	float Duration,
	float TimeDilation,
	bool bAffectInstigator)
{
	UAbilityTask_HitStop* Task = NewAbilityTask<UAbilityTask_HitStop>(OwningAbility);
	Task->StopDuration = Duration;
	Task->StopDilation = FMath::Clamp(TimeDilation, 0.f, 1.f);
	Task->bAffectInstigatorToo = bAffectInstigator;
	
	return Task;
}

void UAbilityTask_HitStop::Activate()
{
	CachedAvatar = GetAvatarActor();
	CachedInstigator = bAffectInstigatorToo ? AbilitySystemComponent->GetOwnerActor() : nullptr;
	
	// 피격자 없으면 즉시 종료 
	if (!CachedAvatar.IsValid())
	{
		OnFinished.Broadcast();
		EndTask();
		
		return;
	}
	
	if (ACharacter* Character = Cast<ACharacter>(CachedAvatar.Get()))
		CachedAvatarMesh = Character->GetMesh();
	
	if (bAffectInstigatorToo && CachedInstigator.IsValid() && CachedInstigator != CachedAvatar)
	{
		if (ACharacter* Character = Cast<ACharacter>(CachedInstigator.Get()))
			CachedInstigatorMesh = Character->GetMesh();
	}
		
	SetMeshAnimRate(CachedAvatarMesh.Get(), StopDilation);
	SetMeshAnimRate(CachedInstigatorMesh.Get(), StopDilation);

	// 복구 타이머 
	GetWorld()->GetTimerManager().SetTimer(
		RecoveryTimer, this, &UAbilityTask_HitStop::RecoverTime, StopDuration, false);
	
	KO_LOG(GAS, Warning, TEXT("HitStop Task : Timer Start."));
}

void UAbilityTask_HitStop::OnDestroy(bool bInOwnerFinished)
{
	KO_LOG(GAS, Warning, TEXT("HitStop Task : OnDestroy."));
	
	GetWorld()->GetTimerManager().ClearTimer(RecoveryTimer);
	
	SetMeshAnimRate(CachedAvatarMesh.Get(), 1.f);
	SetMeshAnimRate(CachedInstigatorMesh.Get(), 1.f);

	OnFinished.Broadcast();
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_HitStop::SetMeshAnimRate(USkeletalMeshComponent* Mesh, float Rate)
{
	if (Mesh) Mesh->GlobalAnimRateScale = Rate;
}

void UAbilityTask_HitStop::RecoverTime()
{
	EndTask();
	
	KO_LOG(GAS, Warning, TEXT("HitStop Task : Timer End."));
}
