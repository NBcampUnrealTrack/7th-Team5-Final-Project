#include "KOMovementSet.h"
#include "Net/UnrealNetwork.h"

UKOMovementSet::UKOMovementSet()
{
}

void UKOMovementSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	if (Attribute == GetMoveSpeedAttribute())
		NewValue = FMath::Max(NewValue, 0.f);
	
	if (Attribute == GetMaxMoveSpeedAttribute())
		NewValue = FMath::Max(NewValue, 1.f);
	
	if (Attribute == GetJumpStrengthAttribute())
		NewValue = FMath::Max(NewValue, 0.f);
}

void UKOMovementSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetMoveSpeedAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMoveSpeed());
	
	if (Attribute == GetMaxMoveSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
		AdjustCurrentForMaxChange(
			MoveSpeed,
			MaxMoveSpeed, 
			NewValue,
			GetMoveSpeedAttribute()
		);
	}
	
	if (Attribute == GetJumpStrengthAttribute())
		NewValue = FMath::Max(NewValue, 0.f);
}

void UKOMovementSet::PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const
{
	Super::PostAttributeBaseChange(Attribute, OldValue, NewValue);
	
	//TODO: 로깅, 통계 기록, 서버 이벤트 발행 등... 
	if (Attribute == GetMoveSpeedAttribute())
		OnMoveSpeedBaseChanged.Broadcast(OldValue, NewValue);
	
	if (Attribute == GetMaxMoveSpeedAttribute())
		OnMaxMoveSpeedBaseChanged.Broadcast(OldValue, NewValue);
	
	if (Attribute == GetJumpStrengthAttribute())
		OnJumpStrengthBaseChanged.Broadcast(OldValue, NewValue);
}

void UKOMovementSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetMoveSpeedAttribute())
		OnMoveSpeedChanged.Broadcast(OldValue, NewValue);
	
	if (Attribute == GetMaxMoveSpeedAttribute())
		OnMaxMoveSpeedChanged.Broadcast(OldValue, NewValue);
	
	if (Attribute == GetJumpStrengthAttribute())
		OnJumpStrengthChanged.Broadcast(OldValue, NewValue);
}

void UKOMovementSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	// TODO: Effect 태그, Source Actor 등 문맥 접근이 필요한 처리
	// 예: 슬로우 태그 감지 → 이펙트/사운드 트리거 이벤트 발행
}

void UKOMovementSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UKOMovementSet, MoveSpeed,    COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKOMovementSet, MaxMoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKOMovementSet, JumpStrength, COND_None, REPNOTIFY_Always);
}

void UKOMovementSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKOMovementSet, MoveSpeed, OldMoveSpeed);
}

void UKOMovementSet::OnRep_MaxMoveSpeed(const FGameplayAttributeData& OldMaxMoveSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKOMovementSet, MaxMoveSpeed, OldMaxMoveSpeed);
}

void UKOMovementSet::OnRep_JumpStrength(const FGameplayAttributeData& OldJumpStrength)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UKOMovementSet, JumpStrength, OldJumpStrength);
}
