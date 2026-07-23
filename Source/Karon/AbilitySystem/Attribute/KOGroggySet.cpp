#include "AbilitySystem/Attribute/KOGroggySet.h"

#include "GameplayEffectExtension.h"
 
UKOGroggySet::UKOGroggySet()
{
	InitGroggyHealth(100.f);
	InitMaxGroggyHealth(100.f);
	InitGroggyDamage(0.f);
}
 
void UKOGroggySet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
 
	if (Attribute == GetGroggyHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxGroggyHealth());
 
	if (Attribute == GetMaxGroggyHealthAttribute())
		NewValue = FMath::Max(NewValue, 0.f);
}
 
void UKOGroggySet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
 
	if (Attribute == GetGroggyHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxGroggyHealth());
 
	if (Attribute == GetMaxGroggyHealthAttribute())
		AdjustCurrentForMaxChange(
			GroggyHealth, MaxGroggyHealth,
			NewValue,
			GetGroggyHealthAttribute()
		);
}
 
void UKOGroggySet::PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const
{
	Super::PostAttributeBaseChange(Attribute, OldValue, NewValue);
 
	if (Attribute == GetGroggyHealthAttribute())
	{
		OnGroggyHealthBaseChanged.Broadcast(OldValue, NewValue);
		
		if (NewValue <= 0.f && OldValue > 0.f)
		{
			OnGroggyTriggered.Broadcast();
		}
	}
 
	if (Attribute == GetMaxGroggyHealthAttribute())
		OnMaxGroggyHealthBaseChanged.Broadcast(OldValue, NewValue);
	
}
 
void UKOGroggySet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	// 메타 어트리뷰트는 처리 제외
	if (Attribute == GetGroggyDamageAttribute()) 	
	{
		return;
	}
 
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
 
	if (Attribute == GetGroggyHealthAttribute())
		OnGroggyHealthChanged.Broadcast(OldValue, NewValue);
 
	if (Attribute == GetMaxGroggyHealthAttribute())
		OnMaxGroggyHealthChanged.Broadcast(OldValue, NewValue);
}
 
void UKOGroggySet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
 
	if (Data.EvaluatedData.Attribute == GetGroggyDamageAttribute())
	{
		HandleGroggyDamage(Data);
	}
}
 
void UKOGroggySet::HandleGroggyDamage(const FGameplayEffectModCallbackData& Data)
{
	const float DamageAmount = GetGroggyDamage();
 
	const float NewGroggyHealth = FMath::Clamp(
		GetGroggyHealth() - DamageAmount,
		0.f,
		GetMaxGroggyHealth()
	);
 
	SetGroggyHealth(NewGroggyHealth);
	SetGroggyDamage(0.f);
}
 
