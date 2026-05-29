#include "AbilityTask_HitStop.h"

#include "AbilitySystemComponent.h"

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
	CachedInstigator = bAffectInstigatorToo
	  ? AbilitySystemComponent->GetOwnerActor()
	  : nullptr;
	
	// 피격자 없으면 즉시 종료 
	if (!CachedAvatar.IsValid())
	{
		OnFinished.Broadcast();
		EndTask();
		
		return;
	}

	// 피격자 시간 정지 
	CachedAvatar->CustomTimeDilation = StopDilation;
	
	// 공격자도 시간 정지 
	if (CachedInstigator.IsValid() && CachedInstigator != CachedAvatar)
		CachedInstigator->CustomTimeDilation = StopDilation;

	// 복구 타이머 
	GetWorld()->GetTimerManager().SetTimer(
		RecoveryTimer, this, &UAbilityTask_HitStop::RecoverTime, StopDuration, false);
}

void UAbilityTask_HitStop::RecoverTime()
{
	if (CachedAvatar.IsValid())
		CachedAvatar->CustomTimeDilation = 1.0f;
	
	if (CachedInstigator.IsValid() && CachedInstigator != CachedAvatar)
		CachedInstigator->CustomTimeDilation = 1.0f;
	
	OnFinished.Broadcast();
	EndTask();
}

// FHitStopFinishedDelegate::FHitStopFinishedDelegate()
// {
// }
//
// FHitStopFinishedDelegate::FHitStopFinishedDelegate(const TMulticastScriptDelegate<>& InMulticastScriptDelegate)
// {
// }

