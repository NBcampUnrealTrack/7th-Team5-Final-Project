#include "KOGA_Utility_UnequipWeapon.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "AbilitySystem/Tag/State/KOGameplayTags_State.h"
#include "Character/KOCharacterBase.h"
#include "Component/Inventory/KOEquipmentComponent.h"

UKOGA_Utility_UnequipWeapon::UKOGA_Utility_UnequipWeapon()
{
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = KOGameplayTags::Event_Weapon_Unequip;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

bool UKOGA_Utility_UnequipWeapon::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
		return false; 
	
	AKOCharacterBase* Character = Cast<AKOCharacterBase>(GetAvatarCharacter());
	if (!Character) return false; 
	
	UKOEquipmentComponent* EquipComponent = Character->GetEquipmentComponent();
	if (!EquipComponent) return false; 
	
	return EquipComponent->HasWeapon();
}

void UKOGA_Utility_UnequipWeapon::ActivateAbility(
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
	if (!EquipComponent)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return; 
	}
	
	GetASC()->RemoveLooseGameplayTag(KOGameplayTags::State_Character_WeaponDrawn);
}
