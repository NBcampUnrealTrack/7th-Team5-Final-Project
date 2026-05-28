#include "KOGA_Attack_Light.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "GameplayTagContainer.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Log/KOLogCategory.h"

UKOGA_Attack_Light::UKOGA_Attack_Light()
{
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Attack_Light));
}

void UKOGA_Attack_Light::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	bIsComboWindowOpen = false;
	BufferedInput = EAttackInputType::None;
	CurrentComboRowName = NAME_None;
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (!ComboDataTable || InitialComboRowName.IsNone())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	PlayComboMontage(InitialComboRowName);
	UE_LOG(Log_KNT, Warning, TEXT("Play Init Montage"));
}

void UKOGA_Attack_Light::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	bIsComboWindowOpen = false;
	BufferedInput = EAttackInputType::None;
	CurrentComboRowName = NAME_None;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_Attack_Light::InputPressed(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	
	if (bIsComboWindowOpen)
	{
		BufferedInput = EAttackInputType::Light;
	}
}

void UKOGA_Attack_Light::OnGameplayEventReceived(FGameplayEventData Payload)
{
	if (Payload.EventTag == AttackEventTag)
	{
		Super::OnGameplayEventReceived(Payload);	
		return;
	}
	
	if (Payload.EventTag == KOGameplayTags::Event_Combo_Window_Open)
	{
		bIsComboWindowOpen = true;

		
		return;
	}
	
	// if (Payload.EventTag == KOGameplayTags::Event_Input_Heavy)
	// {
	// 	BufferedInput = EAttackInputType::Heavy;
	// 	return;
	// }
	
	// if (Payload.EventTag == KOGameplayTags::Event_Input_Light)
	// {
	// 	BufferedInput = EAttackInputType::Light;
	// 	return;
	// }
	
	if (Payload.EventTag == KOGameplayTags::Event_Combo_Window_Close)
	{
		bIsComboWindowOpen = false;
		FString Context = TEXT("Combo Branch Check");
		FKOComboActionData* CurrentData = ComboDataTable->FindRow<FKOComboActionData>(CurrentComboRowName, Context);
		
		if (CurrentData)
		{
			FName NextRowName = NAME_None;
			switch(CurrentCombo)
			{
			case 1:
				NextRowName = CurrentData->NextLightRow;
				CurrentCombo++;
				break;
			case 2:
				NextRowName = CurrentData->NextLightRow;
				CurrentCombo++;
				break;
			}
			if (!NextRowName.IsNone())
			{
				BufferedInput = EAttackInputType::None;
				PlayComboMontage(NextRowName);
				UE_LOG(Log_KNT, Warning, TEXT("Play %d Montage"), CurrentCombo);
				if (CurrentCombo == MaxCombo)
				{
					CurrentCombo = 1;
				}
				return;
			}
		}
		
		// if (BufferedInput != EAttackInputType::None)
		// {
		// 	FString Context = TEXT("Combo Branch Check");
		// 	FKOComboActionData* CurrentData = ComboDataTable->FindRow<FKOComboActionData>(CurrentComboRowName, Context);
		// 	
		// 	if (CurrentData)
		// 	{
		// 		FName NextRowName = NAME_None;
		// 		
		// 		switch (BufferedInput)
		// 		{
		// 		case EAttackInputType::Light:
		// 			NextRowName = CurrentData->NextLightRow;
		// 			break;
		// 		case EAttackInputType::Heavy:
		// 			NextRowName = CurrentData->NextHeavyRow;
		// 			break;
		// 		default:
		// 			break;
		// 		}
		// 		
		// 		if (!NextRowName.IsNone())
		// 		{
		// 			BufferedInput = EAttackInputType::None;
		// 			PlayComboMontage(NextRowName);
		// 			return;
		// 		}
		// 	}
		// }
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UKOGA_Attack_Light::PlayComboMontage(FName RowName)
{
	FString ContextString = TEXT("Play Combo Montage");
	FKOComboActionData* ComboData = ComboDataTable->FindRow<FKOComboActionData>(RowName, ContextString);
	
	if (!ComboData || !ComboData->ComboMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	CurrentComboRowName = RowName;
	DamageEffectClass = ComboData->DamageEffect;
	
	if (CurrentMontageTask)
	{
		CurrentMontageTask->OnCompleted.RemoveAll(this);
		CurrentMontageTask->OnInterrupted.RemoveAll(this);
		CurrentMontageTask->OnCancelled.RemoveAll(this);
		CurrentMontageTask->OnBlendOut.RemoveAll(this);
		//CurrentMontageTask->EndTask();
	}
	
	CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		ComboData->ComboMontage,
		1.0f, // 나중에 공속으로 변경
		ComboData->MontageSection
	);
	
	CurrentMontageTask->OnCompleted.AddDynamic(this, &UKOGA_Attack_Light::OnComboMontageCompleted);
	CurrentMontageTask->OnInterrupted.AddDynamic(this, &UKOGA_Attack_Light::OnComboMontageCancelled);
	CurrentMontageTask->OnCancelled.AddDynamic(this, &UKOGA_Attack_Light::OnComboMontageCancelled);
	CurrentMontageTask->OnBlendOut.AddDynamic(this, &UKOGA_Attack_Light::OnComboMontageBlendOut);
	
	CurrentMontageTask->ReadyForActivation();
}

void UKOGA_Attack_Light::OnComboMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_Attack_Light::OnComboMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Attack_Light::OnComboMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
