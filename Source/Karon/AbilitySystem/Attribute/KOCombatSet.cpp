#include "KOCombatSet.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/Tag/Data/KOGameplayTags_Data.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "AbilitySystem/Tag/State/KOGameplayTags_State.h"
#include "Utility/Messaging/KOMessageTypes.h"

UKOCombatSet::UKOCombatSet()
{
	InitAttackSpeed(1.f); 
	InitCritChance(0.5f);     
	InitCritMultiplier(1.5f);   
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
	
	if (Attribute == GetCritChanceAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, 1.f);
		
	if (Attribute == GetCritMultiplierAttribute())
		NewValue = FMath::Max(NewValue, 1.f);
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
	
}

void UKOCombatSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetOverClockGaugeAttribute())
	{
		SetOverClockGauge(FMath::Clamp(GetOverClockGauge(), 0.0f, GetMaxOverClockGauge()));
		
		UE_LOG(LogTemp, Warning, TEXT("[Overclock] 현재 게이지: %f / %f"), GetOverClockGauge(), GetMaxOverClockGauge());
		
		FKOOverclockProgressBarMessage OverclockMessage;
		OverclockMessage.Percent=GetOverClockGauge()/GetMaxOverClockGauge();
			
		UGMRouterSubsystem::BroadcastMessage(GetWorld(),
			KOGameplayTags::Event_SyncOverclockProgressBar,
			FInstancedStruct::Make(OverclockMessage));
		
		if (GetOverClockGauge() >= GetMaxOverClockGauge())
		{
			UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
			
			FGameplayTag OverClockTag = KOGameplayTags::State_Character_OverClock;
			if (ASC && !ASC->HasMatchingGameplayTag(OverClockTag))
			{
				AActor* AvatarActor = Data.Target.GetAvatarActor();
				if (!AvatarActor) return;
				
				FGameplayEventData Payload;
				Payload.Instigator = AvatarActor;
				Payload.Target = AvatarActor;
				
				FGameplayTag EventTag = KOGameplayTags::Event_OverClock_Start;
				
				UE_LOG(LogTemp, Error, TEXT("[Overclock] 게이지 MAX. 오버클럭 실행 이벤트를 발송"));
				
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(AvatarActor, EventTag, Payload);
			}
		}
	}
}

