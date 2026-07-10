// Fill out your copyright notice in the Description page of Project Settings.


#include "KOTelemetrySubsystem.h"

#include "AbilitySystemComponent.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayTagContainer.h"
#include "GMRouterSubsystem.h"
#include "AbilitySystem/Tag/Data/KOGameplayTags_Data.h"
#include "AbilitySystem/Tag/Effect/KOGameplayTags_Effect.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "Character/Hero/KOHeroCharacter.h"
#include "Data/Type/KOTelemetryType.h"
#include "Items/KOItemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/Messaging/KOMessageTypes.h"

void UKOTelemetrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
#if !UE_BUILD_SHIPPING
	SessionId = FGuid::NewGuid().ToString(EGuidFormats::Digits);

	// 워커 스레드 기동 (DB 오픈 + 테이블 생성은 워커 내부에서)
	const FString DbPath = FPaths::ProjectSavedDir() /
		TEXT("Telemetry") / FString::Printf(TEXT("session_%s.db"), *SessionId);
	Writer = MakeUnique<FKOTelemetryWriter>(DbPath, EventQueue);

	// Combat 구독
	FGameplayMessageCallback CombatCallback;
	CombatCallback.BindDynamic(this, &UKOTelemetrySubsystem::HandleDamageReceived);
	CombatHandle = UGMRouterSubsystem::Subscribe(GetWorld(),KOGameplayTags::Event_Telemetry_Combat, CombatCallback);
	
	// 드랍 아이템 구독
	FGameplayMessageCallback DropCallback;
	DropCallback.BindDynamic(this, &UKOTelemetrySubsystem::HandleDropItem);
	DropHandle = UGMRouterSubsystem::Subscribe(GetWorld(),KOGameplayTags::Event_DropItem, DropCallback);
	
#endif
}

void UKOTelemetrySubsystem::Deinitialize()
{
	#if !UE_BUILD_SHIPPING
		Writer.Reset();             
	#endif
		Super::Deinitialize();
}

void UKOTelemetrySubsystem::HandleDamageReceived(FGameplayTag Channel, const FInstancedStruct& Payload)
{
	if (const FKOTelemetryCombatMessage* Msg = Payload.GetPtr<FKOTelemetryCombatMessage>())
	{
		FKOTelemetryEvent E;
		E.SessionId = SessionId;
		E.GameTime  = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
		E.EventType = TEXT("Combat");
		E.Value     = Msg->Value;
		E.EnemyTag  = Msg->EnemyTag;
		E.EnemyLevel = Msg->EnemyLevel;
		E.AbilityName = Msg->AbilityName;
		E.Position = Msg->Position;
		E.HealthPercentAfter=Msg->HealthPercentAfter;
		Enqueue(E);
	}
}


void UKOTelemetrySubsystem::HandleDropItem(FGameplayTag Channel, const FInstancedStruct& Payload)
{
	if (const FKODropItemMessage* Msg = Payload.GetPtr<FKODropItemMessage>())
	{
		FKOTelemetryEvent E;
		E.SessionId = SessionId;
		E.GameTime  = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
		E.EventType = TEXT("ItemDrop");
		E.Value     = Msg->Count;
		E.EnemyTag  = Msg->EnemyTag;
		E.EnemyLevel = Msg->EnemyLevel;
		E.Extra     = UKOItemLibrary::GetDisplayName(this, EKOSlotKind::Item, Msg->ItemId).ToString();
		Enqueue(E);
	}
}

void UKOTelemetrySubsystem::Enqueue(const FKOTelemetryEvent& Event)
{
	EventQueue.Enqueue(Event);
}
