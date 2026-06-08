#include "KOMovementSet.h"

UKOMovementSet::UKOMovementSet()
{
	//TODO: 나중에 DDD로 전환
	InitMaxWalkSpeed(400.f);
	InitMaxAcceleration(800.f); // 800 300
	InitGroundFriction(5.f); // Strafe : 5 / Sprint : 3
	InitBrakingDecelerationWalking(2000.f); // Strafe : 2000 / Sprint : 500
	InitMaxWalkSpeedCrouched(225); 
	InitJumpStrength(600.f);
}

void UKOMovementSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	if (Attribute == GetMaxWalkSpeedAttribute())
		NewValue = FMath::Max(NewValue, 10.f);
	
	if (Attribute == GetMaxAccelerationAttribute())
		NewValue = FMath::Max(NewValue, 1.f);
	
	if (Attribute == GetGroundFrictionAttribute())
		NewValue = FMath::Max(NewValue, 1.f);
	
	if (Attribute == GetBrakingDecelerationWalkingAttribute())
		NewValue = FMath::Max(NewValue, 0.f);
	
	if (Attribute == GetMaxWalkSpeedCrouchedAttribute())
		NewValue = FMath::Max(NewValue, 0.f);
	
	if (Attribute == GetJumpStrengthAttribute())
		NewValue = FMath::Max(NewValue, 1.f);
}

void UKOMovementSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetMaxWalkSpeedAttribute())
		NewValue = FMath::Max(NewValue, 10.f);
	
	if (Attribute == GetMaxAccelerationAttribute())
		NewValue = FMath::Max(NewValue, 1.f);
	
	if (Attribute == GetGroundFrictionAttribute())
		NewValue = FMath::Max(NewValue, 1.f);
	
	if (Attribute == GetBrakingDecelerationWalkingAttribute())
		NewValue = FMath::Max(NewValue, 0.f);
	
	if (Attribute == GetMaxWalkSpeedCrouchedAttribute())
		NewValue = FMath::Max(NewValue, 0.f);
	
	if (Attribute == GetJumpStrengthAttribute())
		NewValue = FMath::Max(NewValue, 1.f);
}

void UKOMovementSet::PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const
{
	Super::PostAttributeBaseChange(Attribute, OldValue, NewValue);
	
	//TODO: 로깅, 통계 기록, 서버 이벤트 발행 등... 

}

void UKOMovementSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetMaxWalkSpeedAttribute())
		OnMaxWalkSpeedBaseChanged.Broadcast(OldValue, NewValue);
	
	if (Attribute == GetMaxAccelerationAttribute())
		OnMaxWalkSpeedBaseChanged.Broadcast(OldValue, NewValue);
	
	if (Attribute == GetGroundFrictionAttribute())
		OnGroundFrictionChanged.Broadcast(OldValue, NewValue);
	
	if (Attribute == GetBrakingDecelerationWalkingAttribute())
		OnBrakingDecelerationWalkingChanged.Broadcast(OldValue, NewValue);
	
	if (Attribute == GetMaxWalkSpeedCrouchedAttribute())
		OnMaxWalkSpeedCrouchedChanged.Broadcast(OldValue, NewValue);
	
	if (Attribute == GetJumpStrengthAttribute())
		OnJumpStrengthChanged.Broadcast(OldValue, NewValue);
}

void UKOMovementSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	// TODO: Effect 태그, Source Actor 등 문맥 접근이 필요한 처리
	// 예: 슬로우 태그 감지 → 이펙트/사운드 트리거 이벤트 발행
}
