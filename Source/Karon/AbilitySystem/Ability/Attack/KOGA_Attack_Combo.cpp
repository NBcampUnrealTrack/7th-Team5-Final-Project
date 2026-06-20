#include "KOGA_Attack_Combo.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Data/KOComboActionData.h"
#include "Utility/Log/KOLogManager.h"

UKOGA_Attack_Combo::UKOGA_Attack_Combo()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Attack_Heavy));
}

void UKOGA_Attack_Combo::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	ACharacter* Character = GetAvatarCharacter();
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (!ComboDataTable)
	{
		KO_LOG(GAS, Warning, TEXT("[%s] ComboDataTable이 블루프린트에 설정되지 않음"), *GetName()); 
		
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (FKOComboActionData* ComboData = ComboDataTable->FindRow<FKOComboActionData>(WeaponRowName, TEXT("ComboDataContext")))
	{
		ComboMontage = ComboData->LightAttackMontage.Get();
		MaxComboCount = ComboData->MaxLightComboCount;
	}
	
	CurrentComboIndex = 1;
	bIsComboQueued = false;
	bIsInputBufferOpen = false;
	CurrentMontageTask = nullptr;
	
	UAbilityTask_WaitGameplayEvent* HitTask = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Hit);
	
	HitTask->EventReceived.AddDynamic(this, &ThisClass::OnHitEventReceived);
	HitTask->ReadyForActivation();
	
	
	UAbilityTask_WaitGameplayEvent* InputEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Combo_EnableInput);
	
	InputEventTask->EventReceived.AddDynamic(this, &ThisClass::UKOGA_Attack_Combo::OnInputBufferOpened);
	InputEventTask->ReadyForActivation();
	
	
	UAbilityTask_WaitGameplayEvent* ComboEventTask = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Combo_Check);
	
	ComboEventTask->EventReceived.AddDynamic(this, &ThisClass::OnComboWindowReceived);
	ComboEventTask->ReadyForActivation();
	

	PlayNextComboSection();
}

void UKOGA_Attack_Combo::InputPressed(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	UE_LOG(LogTemp, Warning, TEXT("어빌리티 내부에서 클릭 입력 감지 현재 타수: %d"), CurrentComboIndex);

	if (bIsInputBufferOpen && CurrentComboIndex < MaxComboCount)
	{
		bIsComboQueued = true;
		UE_LOG(LogTemp, Warning, TEXT("bIsComboQueued true"));
	}
}

void UKOGA_Attack_Combo::PlayNextComboSection()
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
	bIsInputBufferOpen = false;
	
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

void UKOGA_Attack_Combo::OnComboWindowReceived(FGameplayEventData Payload)
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

void UKOGA_Attack_Combo::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,true,false);
}

void UKOGA_Attack_Combo::OnHitEventReceived(FGameplayEventData Payload)
{
	SendAttackEventsToTarget(&Payload);
	ApplyHitEffects(&Payload);
}

void UKOGA_Attack_Combo::OnInputBufferOpened(FGameplayEventData Payload)
{
	bIsInputBufferOpen = true;
}

