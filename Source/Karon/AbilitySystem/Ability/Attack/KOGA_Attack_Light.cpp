#include "KOGA_Attack_Light.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "GameplayTagContainer.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/KOAbilitySystemComponent.h"

UKOGA_Attack_Light::UKOGA_Attack_Light()
{
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Attack_Light));
}

void UKOGA_Attack_Light::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
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
	
	PlayComboMontage(InitialComboRowName);
	UE_LOG(LogTemp, Warning, TEXT("Play Init Montage"));
}

void UKOGA_Attack_Light::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	bIsComboWindowOpen = false;
	BufferedInput = EAttackInputType::None;
	CurrentComboRowName = NAME_None;
}

void UKOGA_Attack_Light::InputPressed(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	
	if (bIsComboWindowOpen)
	{
		BufferedInput = EAttackInputType::Light;
	}
}

void UKOGA_Attack_Light::OnGameplayEventReceived(FGameplayEventData Payload)
{
	if (Payload.EventTag == KOGameplayTags::Event_Hit)
	{
		const AActor* TargetActor = Payload.Target;
		if (!TargetActor || !DamageEffectClass)
		{
			return;
		}
		
		UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
		if (!TargetASC)
		{
			return;
		}
		
		FGameplayEffectContextHandle EffectContext = CurrentActorInfo->AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());
		
		if (Payload.TargetData.Num() > 0)
		{
			const FHitResult* HitResult = Payload.TargetData.Get(0)->GetHitResult();
			if (HitResult)
			{
				EffectContext.AddHitResult(*HitResult, true);
			}
		}
		
		FGameplayEffectSpecHandle SpecHandle = CurrentActorInfo->AbilitySystemComponent->MakeOutgoingSpec(
			DamageEffectClass,
			GetAbilityLevel(),
			EffectContext
		);
		
		if (SpecHandle.IsValid())
		{
			CurrentActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			UE_LOG(LogTemp, Warning, TEXT("Target: %s, GE: %s"), 
		*TargetActor->GetName(), *DamageEffectClass->GetName());
		}
		
		return;
	}
	
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
				UE_LOG(LogTemp, Warning, TEXT("Play %d Montage"), CurrentCombo);
				if (CurrentCombo == MaxCombo)
				{
					CurrentCombo = 1;
				}
				return;
			}
		}

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
