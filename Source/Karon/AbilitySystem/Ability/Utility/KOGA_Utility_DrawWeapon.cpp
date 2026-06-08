#include "KOGA_Utility_DrawWeapon.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/KOCharacterBase.h"
#include "Component/Inventory/KOEquipmentComponent.h"
#include "Data/Equipment/KOWeaponDefinition.h"
#include "Utility/Log/KOLogManager.h"


UKOGA_Utility_DrawWeapon::UKOGA_Utility_DrawWeapon()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Utility_DrawWeapon));

	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_WeaponDrawn);
	
}

void UKOGA_Utility_DrawWeapon::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AKOCharacterBase* Character = Cast<AKOCharacterBase>(GetAvatarCharacter());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UKOEquipmentComponent* EquipComponent = Character->GetEquipmentComponent();
	if (!EquipComponent || !EquipComponent->HasWeapon())
	{
		KO_LOGS(GAS, Weapon, Warning, TEXT("Character has not Equipment Component or Weapon"));
		
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UKOWeaponDefinition* Config = EquipComponent->GetCurrentWeaponConfig();
	if (!Config)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAnimMontage* Montage = Config->DrawMontage;
	if (!Montage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;	
	}

	UAbilityTask_WaitGameplayEvent* EventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			KOGameplayTags::Event_Weapon_Draw
		);

	EventTask->EventReceived.AddDynamic(this, &ThisClass::OnDrawAttachEvent);
	EventTask->ReadyForActivation();

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			Montage,
			1.0f,
			NAME_None,
			true
		);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);

	MontageTask->ReadyForActivation();
}

void UKOGA_Utility_DrawWeapon::OnMontageCompleted()
{
	// 몽타주 완료 = 뽑기 성공 → WeaponDrawn 상태 태그 추가
	if (UAbilitySystemComponent* ASC = GetASC())
	{
		ASC->AddLooseGameplayTag(KOGameplayTags::State_Character_WeaponDrawn);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_Utility_DrawWeapon::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Utility_DrawWeapon::OnDrawAttachEvent(FGameplayEventData Payload)
{
	AKOCharacterBase* Character = Cast<AKOCharacterBase>(GetAvatarCharacter());
	if (!Character) return;

	UKOEquipmentComponent* EquipComponent = Character->GetEquipmentComponent();
	if (!EquipComponent) return;

	EquipComponent->DrawWeapon();
}
