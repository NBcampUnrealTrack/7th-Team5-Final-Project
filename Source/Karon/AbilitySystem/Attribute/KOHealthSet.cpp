#include "KOHealthSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UKOHealthSet::UKOHealthSet()
{
	InitDamage(0.f);
	InitHealing(0.f);
}

// Base 값 변경 전 - Clamp 만 
void UKOHealthSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());

	if (Attribute == GetMaxHealthAttribute())
		NewValue = FMath::Max(NewValue, 0.f);
}

// Current값 변경 전 - 클램핑 + Max변경 시 비율 조정
void UKOHealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	
	if (Attribute == GetMaxHealthAttribute())
		AdjustCurrentForMaxChange(
			Health, MaxHealth,
			NewValue,
			GetHealthAttribute()
		);	
}

// Base 영구 변경 후 - 레벨업/장비/세이브
void UKOHealthSet::PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const
{
	Super::PostAttributeBaseChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetHealthAttribute())
		OnHealthBaseChanged.Broadcast(OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute())
		OnMaxHealthBaseChanged.Broadcast(OldValue, NewValue);
}

// Current값 변경 후 - UI/이벤트
void UKOHealthSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetHealthAttribute())
		OnHealthChanged.Broadcast(OldValue, NewValue);
	
	if (Attribute == GetMaxHealthAttribute())
		OnMaxHealthChanged.Broadcast(OldValue, NewValue);
}

void UKOHealthSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	Super::PostGameplayEffectExecute(Data);
	
	FKOEffectContext Context = CacheEffectContext(Data);
	
	// Handle Damage 
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		float DamageAmount = GetDamage();
		
		// TODO: 무적 체크 
		// if (Context.TargetASC->HasMatchingGameplayTag(TAG_State_Invincible))
		// 	DamageAmount = 0.f;
		
		float NewHealth = FMath::Clamp(
			GetHealth() - DamageAmount, 
			0.f, 
			GetMaxHealth()
		);
		
		SetHealth(NewHealth);
		SetDamage(0.f); // 메타 데이터 초기화 
		
		if (NewHealth <= 0.f)
		{
			// TODO: OnDeath 이벤트  
		}
	}
	
	// Handle Heal
	if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		float NewHealth = FMath::Clamp(
			GetHealth() + GetHealing(), 
			0.f,
			GetMaxHealth()
		);
		
		SetHealth(NewHealth);
		SetHealing(0.f); // 메타 데이터 초기화 
	}
}

void UKOHealthSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// 일반 어트리뷰트만 복제 
	DOREPLIFETIME_CONDITION_NOTIFY(UKOHealthSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKOHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UKOHealthSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKOHealthSet, Health, OldHealth);
}

void UKOHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKOHealthSet, MaxHealth, OldMaxHealth);
}
