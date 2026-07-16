#include "KOGA_Utility_SheatheWeapon.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/KOCharacterBase.h"
#include "Component/Inventory/KOEquipmentComponent.h"
#include "Data/Equipment/KOWeaponDefinition.h"


UKOGA_Utility_SheatheWeapon::UKOGA_Utility_SheatheWeapon()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Utility_SheatheWeapon));
	
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Character_Drawing);
	// 뽑혀 있을 때만 활성화 가능
	ActivationRequiredTags.AddTag(KOGameplayTags::State_Character_WeaponDrawn);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Attacking);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Guard_Blocking);
}

void UKOGA_Utility_SheatheWeapon::ActivateAbility(
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
	if (!Character) return;

	UKOEquipmentComponent* EquipComponent = Character->GetEquipmentComponent();
	if (!EquipComponent || !EquipComponent->IsWeaponDrawn()) return;

	UKOWeaponDefinition* Config = EquipComponent->GetCurrentWeaponConfig();
	if (!Config) return;

	UAnimMontage* Montage = Config->WeaponAnimationSet.SheatheMontage;
	if (!Montage) return;

	UAbilityTask_WaitGameplayEvent* EventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			KOGameplayTags::Event_Weapon_Sheathe
		);

	EventTask->EventReceived.AddDynamic(this, &ThisClass::OnSheatheAttachEvent);
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

void UKOGA_Utility_SheatheWeapon::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	GetASC()->RemoveLooseGameplayTag(KOGameplayTags::State_Character_WeaponDrawn);
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_Utility_SheatheWeapon::OnMontageCompleted()
{
	// 넣기 완료 → WeaponDrawn 상태 태그 제거
	if (UAbilitySystemComponent* ASC = GetASC())
	{
		ASC->RemoveLooseGameplayTag(KOGameplayTags::State_Character_WeaponDrawn);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_Utility_SheatheWeapon::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Utility_SheatheWeapon::OnSheatheAttachEvent(FGameplayEventData Payload)
{
	AKOCharacterBase* Character = Cast<AKOCharacterBase>(GetAvatarCharacter());
	if (!Character) return;

	UKOEquipmentComponent* EquipComponent = Character->GetEquipmentComponent();
	if (!EquipComponent) return;

	EquipComponent->SheatheWeapon();
}
