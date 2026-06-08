#include "KOGrantSet.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
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
	
	// Passive 어빌리티 부여 
	for (const FKOPassiveAbilityEntry& Entry : PassiveAbilities)
	{
		if (!IsValid(Entry.Ability)) continue;

		FGameplayAbilitySpec Spec(Entry.Ability, Entry.AbilityLevel);

		OutHandles.AbilityHandles.Add(ASC->GiveAbility(Spec));

		KO_LOG(GAS, Log, TEXT("[Passive] Ability  | %-30s | Lv.%d "),
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

