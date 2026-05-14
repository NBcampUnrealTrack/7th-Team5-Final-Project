#include "KOCombatSet.h"

#include "Net/UnrealNetwork.h"

UKOCombatSet::UKOCombatSet()
{
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
}

void UKOCombatSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetAttackSpeedAttribute())
		NewValue = FMath::Clamp(NewValue, 0.1f, 10.f);
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
	{
		OnAttackSpeedChanged.Broadcast(OldValue, NewValue);

		// TODO:몽타주 재생 속도 동기화
		// if (ACharacter* Character = Cast<ACharacter>(GetOwningAbilitySystemComponent()->GetAvatarActor()))
		// {
		// 	Character->GetMesh()->GlobalAnimRateScale = NewValue;
		// }
	}
}

void UKOCombatSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void UKOCombatSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UKOCombatSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKOCombatSet, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKOCombatSet, AttackSpeed, COND_None, REPNOTIFY_Always);
}

void UKOCombatSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKOCombatSet, AttackPower, OldAttackPower);
}

void UKOCombatSet::OnRep_Defense(const FGameplayAttributeData& OldDefense)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKOCombatSet, Defense, OldDefense);
}

void UKOCombatSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKOCombatSet, AttackSpeed, OldAttackSpeed);
}
