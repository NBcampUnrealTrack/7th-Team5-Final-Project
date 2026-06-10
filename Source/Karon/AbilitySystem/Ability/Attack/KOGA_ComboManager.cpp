#include "KOGA_ComboManager.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Tag/Data/KOGameplayTags_Data.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "Data/KOComboActionData.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Tag/Input/KOGameplayTags_Input.h"

UKOGA_ComboManager::UKOGA_ComboManager()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UKOGA_ComboManager::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo) || !ComboDataTable)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (TriggerEventData)
	{
		if (TriggerEventData->EventTag == KOGameplayTags::Input_Ability_Attack_Light)
		{
			CurrentComboRowName = FName("Light1");
		}
		else if (TriggerEventData->EventTag == KOGameplayTags::Input_Ability_Attack_Heavy)
		{
			CurrentComboRowName = FName("Heavy1");
		}
	}
	else
	{
		CurrentComboRowName = InitialComboRowName;	
	}	
	
	bIsComboWindowOpen = false;
	BufferedInput = EAttackInputType::None;
	
	
	UAbilityTask_WaitGameplayEvent* WaitOpen = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Combo_Window_Open);
	WaitOpen->EventReceived.AddDynamic(this, &UKOGA_ComboManager::OnComboWindowOpened);
	WaitOpen->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* WaitClose =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Combo_Window_Close);
	WaitClose->EventReceived.AddDynamic(this, &UKOGA_ComboManager::OnComboWindowClosed);
	WaitClose->ReadyForActivation();
	
	
	UAbilityTask_WaitGameplayEvent* WaitLightInput =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Input_Light);
	WaitLightInput->EventReceived.AddDynamic(this, &UKOGA_ComboManager::OnLightInputReceived);
	WaitLightInput->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* WaitHeavyInput =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Input_Heavy);
	WaitHeavyInput->EventReceived.AddDynamic(this, &UKOGA_ComboManager::OnHeavyInputReceived);
	WaitHeavyInput->ReadyForActivation();
	
	SendExecutionEvent(CurrentComboRowName);
}

void UKOGA_ComboManager::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_ComboManager::SendExecutionEvent(FName RowName)
{
	FString Context = TEXT("Combo Manager Excution");
	FKOComboActionData* ComboData = ComboDataTable->FindRow<FKOComboActionData>(RowName, Context);
	
	if (ComboData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ComboManager] '%s'"), *RowName.ToString());
		FGameplayEventData Payload;
		Payload.Instigator = GetAvatarActorFromActorInfo();
		Payload.OptionalObject = ComboData->ComboMontage;
		Payload.OptionalObject2 = ComboData->DamageEffect.Get();
		
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			GetAvatarActorFromActorInfo(),
			KOGameplayTags::Event_Attack_Excute,
			Payload
		);
	}
}

void UKOGA_ComboManager::OnComboWindowOpened(FGameplayEventData Payload)
{
	bIsComboWindowOpen = true;
}

void UKOGA_ComboManager::OnComboWindowClosed(FGameplayEventData Payload)
{
	bIsComboWindowOpen = false;
	
	FString Context = TEXT("Combo Branch Evaluation");
	FKOComboActionData* CurrentData = ComboDataTable->FindRow<FKOComboActionData>(CurrentComboRowName, Context);
	
	if (CurrentData && BufferedInput != EAttackInputType::None)
	{
		FName NextRowName = NAME_None;
		
		switch (BufferedInput)
		{
		case EAttackInputType::Light:
			NextRowName = CurrentData->NextLightRow;
			break;
		case EAttackInputType::Heavy:
			NextRowName = CurrentData->NextHeavyRow;
			break;
		}
		
		if (!NextRowName.IsNone())
		{
			CurrentComboRowName = NextRowName;
			BufferedInput = EAttackInputType::None;
			SendExecutionEvent(CurrentComboRowName);
			return;
		}
		
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_ComboManager::OnLightInputReceived(FGameplayEventData Payload)
{
	if (bIsComboWindowOpen)
	{
		BufferedInput = EAttackInputType::Light;
	}
}

void UKOGA_ComboManager::OnHeavyInputReceived(FGameplayEventData Payload)
{
	if (bIsComboWindowOpen)
	{
		BufferedInput = EAttackInputType::Heavy;
	}
}
