#include "KOGrantSet.h"
#include "AbilitySystemComponent.h"
#include "KOCharacterStatRow.h"
#include "SQLitePreparedStatement.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/Tag/Data/KOGameplayTags_Data.h"
#include "Utility/Log/KOLogManager.h"

void FKOAbilitySetHandles::RemoveFromASC(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;

	for (const FGameplayAbilitySpecHandle& Handle : AbilityHandles)
		ASC->ClearAbility(Handle);
	
	for (const FActiveGameplayEffectHandle& Handle : EffectHandles)
		ASC->RemoveActiveGameplayEffect(Handle);

	AbilityHandles.Reset();
	EffectHandles.Reset();
}

UKOGrantSet::UKOGrantSet(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UKOGrantSet::GiveToAsc(UAbilitySystemComponent* ASC, FKOAbilitySetHandles& OutHandles) 
{
	if (!ASC) return;
	
	// Sub 어빌리티 부여 
	for (const FKOSubAbilityEntry& Entry : SubAbilities)
	{
		if (!IsValid(Entry.Ability)) continue;

		FGameplayAbilitySpec Spec(Entry.Ability, Entry.AbilityLevel);
		
		if (Entry.bStartActivated)
		{
			OutHandles.AbilityHandles.Add(ASC->GiveAbilityAndActivateOnce(Spec));
		}
		else
		{
			OutHandles.AbilityHandles.Add(ASC->GiveAbility(Spec));
		}

		KO_LOG(GAS, Log, TEXT("[Sub] Ability   | %-30s | Lv.%d "),
			*Spec.Ability->GetName(), Entry.AbilityLevel
		);
	}
	
	// Active 어빌리티 부여 
	for (const FKOActiveAbilityEntry& Entry : ActiveAbilities)
	{
		if (!IsValid(Entry.Ability)) continue;

		FGameplayAbilitySpec Spec(Entry.Ability, Entry.AbilityLevel);
		if (Entry.InputTag.IsValid())
			Spec.DynamicAbilityTags.AddTag(Entry.InputTag);

		OutHandles.AbilityHandles.Add(ASC->GiveAbility(Spec));

		KO_LOG(GAS, Log, TEXT("[Active] Ability  | %-30s | Lv.%d | Tag: %s"),
			*Spec.Ability->GetName(), Entry.AbilityLevel,
			Entry.InputTag.IsValid() ? *Entry.InputTag.ToString() : TEXT("None"));
	}
	

	if (AttributeInitializationEffect)
	{
		if (const FKOCharacterStatRow* Row = StatRow.GetRow<FKOCharacterStatRow>(TEXT("GiveToAsc")))
		{
			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			EffectContext.AddSourceObject(this);
        
			FGameplayEffectSpecHandle Handle =
				ASC->MakeOutgoingSpec(AttributeInitializationEffect, 1.0f, EffectContext);
			if (Handle.IsValid())
			{
				Handle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Health_Current, Row->MaxHP);
				Handle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Health_Max, Row->MaxHP);
				
				Handle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Stamina_Current, Row->MaxStamina);
				Handle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Stamina_Max, Row->MaxStamina);
				
				Handle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Combat_AttackPower, Row->Combat.Attack);
				Handle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Combat_AttackSpeed, Row->Combat.AttackSpeed);
				Handle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Combat_CritChance, Row->Combat.CritChance);
				Handle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Combat_CritMultiplier, Row->Combat.CritMultiplier);
				Handle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Combat_Defense, Row->Combat.Defense);
				
				Handle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Movement_WalkSpeed, Row->Movement.MaxWalkSpeed);
				Handle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Movement_MaxAcceleration, Row->Movement.MaxAcceleration);
				Handle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Movement_GroundFriction, Row->Movement.GroundFriction);
				Handle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Movement_BrakingDeceleration, Row->Movement.BrakingDecelerationWalking);
				Handle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Movement_MaxWalkSpeedCrouch, Row->Movement.MaxWalkSpeedCrouched);
				Handle.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Attribute_Movement_JumpStrength, Row->Movement.JumpStrength);
				
				ASC->ApplyGameplayEffectSpecToSelf(*Handle.Data.Get());
			}
		}
	}
	
	// GE 부여 
	for (const FKOEffectEntry& Entry : GrantedEffects)
	{
		if (!IsValid(Entry.Effect)) continue;

		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(
			Entry.Effect, Entry.EffectLevel, Context);

		if (!Spec.IsValid()) continue;

		FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
		OutHandles.EffectHandles.Add(Handle);
		
		KO_LOG(GAS, Log, TEXT("[GrantSet] Effect  | %-30s | Lv.%.1f"),
			*Spec.Data->Def->GetName(), Entry.EffectLevel);
	}
}

