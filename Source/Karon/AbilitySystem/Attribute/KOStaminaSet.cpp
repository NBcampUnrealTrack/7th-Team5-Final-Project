#include "KOStaminaSet.h"

#include "GameplayEffectExtension.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Net/UnrealNetwork.h"

UKOStaminaSet::UKOStaminaSet()
{
	//TODO: DDD로 전환 
	InitStamina(100.f);
	InitMaxStamina(100.f);
	
	// MetaData는 전환 x
	InitStaminaDrain(0.f);
	InitStaminaRegen(0.f);
}

// Base 값 변경 전 - Clamp 만 
void UKOStaminaSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	if (Attribute == GetStaminaAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());

	if (Attribute == GetMaxStaminaAttribute())
		NewValue = FMath::Max(NewValue, 0.f);
}

// Current값 변경 전 - 클램핑 + Max변경 시 비율 조정
void UKOStaminaSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetStaminaAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());

	if (Attribute == GetMaxStaminaAttribute())
		AdjustCurrentForMaxChange(
			Stamina, MaxStamina,
			NewValue,
			GetStaminaAttribute()
		);
}

// Base 영구 변경 후 - 레벨업/장비/세이브
void UKOStaminaSet::PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const
{
	Super::PostAttributeBaseChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetStaminaAttribute())
		OnStaminaBaseChanged.Broadcast(OldValue, NewValue);

	if (Attribute == GetMaxStaminaAttribute())
		OnMaxStaminaBaseChanged.Broadcast(OldValue, NewValue);
}

// Current값 변경 후 - UI/이벤트
void UKOStaminaSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	// 메타 데이터의 경우 처리 x 
	if (Attribute == GetStaminaDrainAttribute() || Attribute == GetStaminaRegenAttribute()) return; 
	
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetStaminaAttribute())
	{    
		UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
		
		if (NewValue <= 0.f)
			ASC->AddLooseGameplayTag(KOGameplayTags::Event_Stamina_Exhausted);
		else if (ASC->HasMatchingGameplayTag(KOGameplayTags::State_Character_StaminaExhausted) &&
		NewValue >= GetMaxStamina()) 
			ASC->RemoveLooseGameplayTag(KOGameplayTags::Event_Stamina_Exhausted);
		
		OnStaminaChanged.Broadcast(OldValue, NewValue);
	}
	
	if (Attribute == GetMaxStaminaAttribute())
		OnMaxStaminaChanged.Broadcast(OldValue, NewValue);
	
}

// GE 실행 완료 후 - 스테미나 사용 / 리젠 
void UKOStaminaSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	auto Context = CacheEffectContext(Data);
	
	// Handle Stamina Drain 
	if (Data.EvaluatedData.Attribute == GetStaminaDrainAttribute())
	{
		float NewStamina = FMath::Clamp(
			GetStamina() - GetStaminaDrain(),
			0.f, GetMaxStamina()
		);
		
		SetStamina(NewStamina);
		SetStaminaDrain(0.f);
	}
	
	// Handle Regen Stamina 
	if (Data.EvaluatedData.Attribute == GetStaminaRegenAttribute())
	{
		float NewStamina = FMath::Clamp(
			GetStamina() + GetStaminaRegen(),
			0.f, GetMaxStamina()
		);
		SetStamina(NewStamina);
		SetStaminaRegen(0.f);
	}
}

