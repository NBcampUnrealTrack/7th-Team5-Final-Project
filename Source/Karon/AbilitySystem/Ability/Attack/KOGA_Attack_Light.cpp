#include "KOGA_Attack_Light.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Data/KOComboActionData.h"

UKOGA_Attack_Light::UKOGA_Attack_Light()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Attack_Light));
	
	CurrentComboIndex = 1;
	MaxComboCount = 1;
	bIsComboQueued = false;
	ComboMontage = nullptr;
}

bool UKOGA_Attack_Light::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, 
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	
	ACharacter* Character = GetAvatarCharacter();
	
	return Character != nullptr;
}

void UKOGA_Attack_Light::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = GetAvatarCharacter();
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (ComboDataTable)
	{
		FKOComboActionData* ComboData = ComboDataTable->FindRow<FKOComboActionData>(WeaponRowName, TEXT("ComboDataContext"));
        
		if (ComboData)
		{
			ComboMontage = ComboData->LightAttackMontage.Get();
			MaxComboCount = ComboData->MaxLightComboCount;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] 데이터 테이블에서 '%s' 행을 찾을 수 없음"), *GetName(), *WeaponRowName.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] ComboDataTable이 블루프린트에 설정되지 않음"), *GetName());
	}
	
	CurrentComboIndex = 1;
	bIsComboQueued = false;
	CurrentMontageTask = nullptr;
	
	UAbilityTask_WaitGameplayEvent* HitTask = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Hit);
	HitTask->EventReceived.AddDynamic(this, &ThisClass::OnHitEventReceived);
	HitTask->ReadyForActivation();
	
	// UAbilityTask_WaitInputPress* InputTask = 
	// 	UAbilityTask_WaitInputPress::WaitInputPress(this, false);
	// if (InputTask)
	// {
	// 	InputTask->OnPress.AddDynamic(this, &ThisClass::OnComboInputPressed);
	// 	InputTask->ReadyForActivation();
	// }
	
	FGameplayTag ComboCheckTag = FGameplayTag::RequestGameplayTag(FName("Event.Combo.Check"));
	UAbilityTask_WaitGameplayEvent* ComboEventTask = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, ComboCheckTag);
	if (ComboEventTask)
	{
		ComboEventTask->EventReceived.AddDynamic(this, &ThisClass::OnComboWindowReceived);
		ComboEventTask->ReadyForActivation();
	}
	
	PlayNextComboSection();
}

void UKOGA_Attack_Light::InputPressed(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	UE_LOG(LogTemp, Warning, TEXT("어빌리티 내부에서 클릭 입력 감지 현재 타수: %d"), CurrentComboIndex);

	if (CurrentComboIndex < MaxComboCount)
	{
		bIsComboQueued = true;
		UE_LOG(LogTemp, Warning, TEXT("bIsComboQueued true"));
	}
}

void UKOGA_Attack_Light::PlayNextComboSection()
{
	if (CurrentMontageTask)
	{
		CurrentMontageTask->OnCompleted.RemoveDynamic(this, &ThisClass::OnMontageEnded);
		CurrentMontageTask->OnBlendOut.RemoveDynamic(this, &ThisClass::OnMontageEnded);
		CurrentMontageTask->OnInterrupted.RemoveDynamic(this, &ThisClass::OnMontageEnded);
		CurrentMontageTask->OnCancelled.RemoveDynamic(this, &ThisClass::OnMontageEnded);
		CurrentMontageTask->EndTask();
	}
	
	FName SectionName = FName(*FString::Printf(TEXT("Attack_%d"), CurrentComboIndex));
	
	CurrentMontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			ComboMontage,
			1.0f,
			SectionName,
			false
		);
	
	CurrentMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageEnded);
	CurrentMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageEnded);
	CurrentMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageEnded);
	CurrentMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageEnded);
	
	CurrentMontageTask->ReadyForActivation();
}
void UKOGA_Attack_Light::OnComboWindowReceived(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("==== [Combo] 몽타주 노티파이 신호 수신 완료! ===="));

	if (bIsComboQueued)
	{
		CurrentComboIndex++;
		bIsComboQueued = false;
       
		UE_LOG(LogTemp, Warning, TEXT("[Combo] 예약된 입력 %d타 애니메이션 재생"), CurrentComboIndex);
       
		PlayNextComboSection();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Combo] 예약된 입력이 없어서 콤보를 종료하고 대기"));
	}
}

void UKOGA_Attack_Light::OnMontageEnded()
{
	EndAbility(
		CurrentSpecHandle, 
		CurrentActorInfo,
		CurrentActivationInfo,
		true,
		false
	);
}

void UKOGA_Attack_Light::OnHitEventReceived(FGameplayEventData Payload)
{
	const UObject* RawPayloadTarget = Payload.Target;
	AActor* TargetActor = Cast<AActor>(const_cast<UObject*>(RawPayloadTarget));
	if (TargetActor && DamageEffectClass)
	{
		ApplyEffectToTarget(TargetActor, DamageEffectClass, GetAbilityLevel());
	}
}


