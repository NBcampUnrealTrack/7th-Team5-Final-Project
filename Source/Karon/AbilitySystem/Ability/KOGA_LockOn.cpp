#include "AbilitySystem/Ability/KOGA_LockOn.h"
#include "Component/KOLockOnComponent.h"
#include "Character/Hero/KOHeroCharacter.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"


UKOGA_LockOn::UKOGA_LockOn()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// Input.Ability.LockOn 태그로 수정
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_LockOn));
	
	// TODO: 락온 하는 동안 캐릭터에 부착할 태그 추가 ex) State_Character_LockOn
	// 인스턴싱 정책 -> PerExecution 이걸로 하면 될듯? 
}

void UKOGA_LockOn::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AKOHeroCharacter* Character = Cast<AKOHeroCharacter>(GetAvatarCharacter());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	UKOLockOnComponent* LockOnComp = Character->GetLockOnComponent();
	if (!LockOnComp)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	//TODO: 락온이 유지되는 동안 어빌리티 유지 
	// 현재는 끄거나 킬때만 잠시 어빌리티가 활성화되는 방식 . 
	// 컴포넌트에서 처리하는걸 여기로 옮기면 굳이 컴포넌트까지도 필요 없어질 가능성 있음. 
	
	// 토글
	if (LockOnComp->IsLockedOn())
		LockOnComp->DeactivateLockOn();
	else
		LockOnComp->ActivateLockOn();

	// 이거 바로 종료 x 
	EndAbility(Handle, ActorInfo, ActivationInfo, false,false);
}

void UKOGA_LockOn::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility,bWasCancelled);
}


