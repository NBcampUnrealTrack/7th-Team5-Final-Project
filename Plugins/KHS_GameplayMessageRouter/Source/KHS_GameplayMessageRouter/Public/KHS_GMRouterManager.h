// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "KHS_GMRouterManager.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FGameplayMessageCallback, FGameplayTag, Channel, const FInstancedStruct&, Payload);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*[GameplayMessage 서브시스템]
 *
 *
 * [송신]
 * 예시 구조체: FDamageMessage
 * FDamageMessage MyData;
 * MyData.Value = 100.f;
 * if (UKHS_GMRouterManager* MessageSubsystem = GetGameInstance()->GetSubsystem<UKHS_GMRouterManager>())
 * {
 *		MessageSubsystem->BroadcastMessage(Tag, FInstancedStruct::Make(MyData));
 * }
 * 
 * [구독]
 * if (UKHS_GMRouterManager* MessageSubsystem = GetGameInstance()->GetSubsystem<UKHS_GMRouterManager>())
 *	{
 *		//해당 태그를 채널로 해서 보냄(예시 : Gameplay.Event.Damage)
 *		FGameplayTag Channel = FGameplayTag::RequestGameplayTag(TEXT("Gameplay.Event.Damage"));		
 *		FGameplayMessageCallback ;
 *		Callback.BindDynamic(this, &UInGameUI::OnCallbackReceived);	
 *		if (UKHS_GMRouterManager* MessageSubsystem = GetGameInstance()->GetSubsystem<UKHS_GMRouterManager>())
 *		{
 *			MessageSubsystem->SubscribeToMessage(Channel, Callback);
 *		}
 *
 *	}
 *	
 * [수신]
 * 예시 구조체: FDamageMessage
 * void OnReceived(FGameplayTag Channel, const FInstancedStruct& Payload)
 * {
 *	if (const FDamageMessage* Data = Payload.GetPtr<FDamageMessage>())
 *	{
 *		float FinalDamage = Data->Value;
 *	}
 * }
 * 
 * [비고]
 * 구조체는 별도의 *Type.h로 관리하는 것을 권장
 * NativeGameplayTag를 사용한다면 위의 예시 코드를 적절히 수정할것.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UCLASS()
class KHS_GAMEPLAYMESSAGEROUTER_API UKHS_GMRouterManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void BroadcastMessage(FGameplayTag Channel, const FInstancedStruct& Payload);
	
	UFUNCTION(BlueprintCallable)
	void SubscribeToMessage(FGameplayTag Channel, const FGameplayMessageCallback& Callback);
	
private:
	TMap<FGameplayTag, TArray<FGameplayMessageCallback>> ListenerMap;
	
};
