#include "KOCombatSet.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Utility/Messaging/KOMessageTypes.h"

UKOCombatSet::UKOCombatSet()
{
	InitAttackSpeed(1.f); 
	InitCritChance(0.5f);     
	InitCritMultiplier(1.5f);
	InitClock(0.f);
	InitClockLimit(100.f);
}

// 최솟값 보장
void UKOCombatSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	if (Attribute == GetAttackPowerAttribute())
		NewValue = FMath::Max(NewValue, 0.f);

	if (Attribute == GetDefenseAttribute())
		NewValue = FMath::Max(NewValue, 0.f);

	if (Attribute == GetAttackSpeedAttribute())
		NewValue = FMath::Clamp(NewValue, 0.1f, 10.f);
	
	if (Attribute == GetClockAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetClockLimit());
}

void UKOCombatSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetAttackSpeedAttribute())
		NewValue = FMath::Clamp(NewValue, 0.1f, 10.f);
	
	else if (Attribute == GetCritChanceAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, 1.f);
		
	else if (Attribute == GetCritMultiplierAttribute())
		NewValue = FMath::Max(NewValue, 1.f);
	
	else if (Attribute == GetClockAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetClockLimit());
}

// Base 영구 변경 (레벨업 / 장비 / 포인트 투자)
void UKOCombatSet::PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const
{
	Super::PostAttributeBaseChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetAttackPowerAttribute())
		OnAttackPowerBaseChanged.Broadcast(OldValue, NewValue);

	if (Attribute == GetDefenseAttribute())
		OnDefenseBaseChanged.Broadcast(OldValue, NewValue);

	if (Attribute == GetAttackSpeedAttribute())
		OnAttackSpeedBaseChanged.Broadcast(OldValue, NewValue);
}

void UKOCombatSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetAttackPowerAttribute())
		OnAttackPowerChanged.Broadcast(OldValue, NewValue);

	if (Attribute == GetDefenseAttribute())
		OnDefenseChanged.Broadcast(OldValue, NewValue);

	if (Attribute == GetAttackSpeedAttribute())
		OnAttackSpeedChanged.Broadcast(OldValue, NewValue);
	
	if (Attribute == GetClockAttribute())
	{
		FKOOverclockProgressBarMessage Message;
		Message.Ratio = GetClock() / GetClockLimit();
		
		UGMRouterSubsystem::BroadcastMessage(
			GetWorld(), KOGameplayTags::Event_SyncOverclockProgressBar, 
			FInstancedStruct::Make(Message)
		);
	}
}

void UKOCombatSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	
	if (Data.EvaluatedData.Attribute == GetClockAttribute())
	{
		UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
		if (!ASC) return;
		
		if (GetClock() >= GetClockLimit() && !ASC->HasMatchingGameplayTag(KOGameplayTags::State_Character_OverClock))
		{
			FGameplayEventData Payload;
			Payload.Instigator = GetOwningActor();
			Payload.Target =  GetOwningActor();
			ASC->HandleGameplayEvent(KOGameplayTags::Event_OverClock_Start, &Payload);
		}
		else if (ASC->HasMatchingGameplayTag(KOGameplayTags::State_Character_OverClock) && GetClock() <= 0.f)
		{
			FGameplayEventData Payload;
			Payload.Instigator = GetOwningActor();
			Payload.Target =  GetOwningActor();
			ASC->HandleGameplayEvent(KOGameplayTags::Event_OverClock_End, &Payload);
		}
	}
}

