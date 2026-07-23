#include "KOGuardSet.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "AbilitySystem/Tag/State/KOGameplayTags_State.h"

UKOGuardSet::UKOGuardSet()
{
	InitGuardHealth(0.f);
	InitMaxGuardHealth(100.f);
	InitGuardDamage(0.f);
}

void UKOGuardSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	if (Attribute == GetGuardHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxGuardHealth());

	if (Attribute == GetMaxGuardHealthAttribute())
		NewValue = FMath::Max(NewValue, 0.f);
}

void UKOGuardSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetGuardHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxGuardHealth());
}

void UKOGuardSet::PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const
{
	Super::PostAttributeBaseChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetGuardHealthAttribute())
	{
		OnGuardHealthBaseChanged.Broadcast(OldValue, NewValue);

		if (NewValue <= 0.f && OldValue > 0.f)
			OnGuardBreakTriggered.Broadcast();
	}

	if (Attribute == GetMaxGuardHealthAttribute())
		OnMaxGuardHealthBaseChanged.Broadcast(OldValue, NewValue);
}

void UKOGuardSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	if (Attribute == GetGuardDamageAttribute())
		return;

	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetGuardHealthAttribute())
		OnGuardHealthChanged.Broadcast(OldValue, NewValue);

	if (Attribute == GetMaxGuardHealthAttribute())
		OnMaxGuardHealthChanged.Broadcast(OldValue, NewValue);
}

void UKOGuardSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetGuardDamageAttribute())
		HandleGuardDamage(Data);
}

void UKOGuardSet::HandleGuardDamage(const FGameplayEffectModCallbackData& Data)
{
	const float DamageAmount = GetGuardDamage();
	const float NewGuardHealth = FMath::Clamp(GetGuardHealth() - DamageAmount, 0.f, GetMaxGuardHealth());

	SetGuardHealth(NewGuardHealth);
	SetGuardDamage(0.f);

	if (NewGuardHealth <= 0.f)
	{
		UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
		if (ASC && ASC->HasMatchingGameplayTag(KOGameplayTags::State_Character_Guard_Blocking))
		{
			FGameplayEventData EventData;
			EventData.Target = ASC->GetAvatarActor();
			EventData.Instigator = Data.EffectSpec.GetContext().GetInstigator();
			
			ASC->HandleGameplayEvent(KOGameplayTags::Event_Guard_Break, &EventData);
		}
		
	}
}
