// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FKOTelemetryWriter.h"
#include "GMRouterSubsystem.h"
#include "Data/Type/KOTelemetryType.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "KOTelemetrySubsystem.generated.h"

struct FGameplayEffectSpec;
struct FActiveGameplayEffectHandle;
class UAbilitySystemComponent;
/**
 * 전투 기록 및 아이템 드랍을 수집하는 서브시스템
 */
UCLASS()
class KARON_API UKOTelemetrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	

private:
	UFUNCTION()
	void HandleDamageReceived(FGameplayTag Channel, const FInstancedStruct& Payload);
	
	UFUNCTION()
	void HandleDropItem(FGameplayTag Channel, const FInstancedStruct& Payload); 
	
	void Enqueue(const FKOTelemetryEvent& Event);
	
private:
	FString SessionId;
	TQueue<FKOTelemetryEvent, EQueueMode::Spsc> EventQueue;
	TUniquePtr<FKOTelemetryWriter> Writer;
	
	UPROPERTY() 
	FGameplayMessageHandle CombatHandle;
	
	UPROPERTY() 
	FGameplayMessageHandle DropHandle;
};
