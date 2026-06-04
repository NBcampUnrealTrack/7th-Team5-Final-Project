#include "KOGA_Utility_EquipWeapon.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/KOCharacterBase.h"
#include "Component/Inventory/KOEquipmentComponent.h"
#include "Data/Equipment/KOWeaponDefinition.h"

UKOGA_Utility_EquipWeapon::UKOGA_Utility_EquipWeapon()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Event_Weapon_Equip 발생 시 자동 활성화
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = KOGameplayTags::Event_Weapon_Equip;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UKOGA_Utility_EquipWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (!TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UKOWeaponDefinition* Def = 
		Cast<UKOWeaponDefinition>(const_cast<UObject*>(TriggerEventData->OptionalObject.Get()));
	
	if (!Def)
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

	EquipComponent->EquipWeapon(Def);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
