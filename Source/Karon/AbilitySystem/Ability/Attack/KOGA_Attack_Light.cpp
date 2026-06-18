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


void UKOGA_Attack_Light::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 수정 2: 코드 순서 변경 : 어지간하면 커밋먼저 
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
	
	// 수정 3: 콤보데이터 가 없으면 종료되지 않음. 
	// if (ComboDataTable)
	// {
	// 	FKOComboActionData* ComboData = ComboDataTable->FindRow<FKOComboActionData>(WeaponRowName, TEXT("ComboDataContext"));
	//        
	// 	if (ComboData)
	// 	{
	// 		ComboMontage = ComboData->LightAttackMontage.Get();
	// 		MaxComboCount = ComboData->MaxLightComboCount;
	// 	}
	// 	else
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("[%s] 데이터 테이블에서 '%s' 행을 찾을 수 없음"), *GetName(), *WeaponRowName.ToString());
	// 		// 콤보 데이터가 없어도 어빌리티가 종료되지않음. 
	// 	}
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[%s] ComboDataTable이 블루프린트에 설정되지 않음"), *GetName()); 
	// 	// 데이터 테이블 없어도 어빌리티가 종료되지않음. 
	// }
	
	if (!ComboDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] ComboDataTable이 블루프린트에 설정되지 않음"), *GetName()); 
		
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (FKOComboActionData* ComboData = ComboDataTable->FindRow<FKOComboActionData>(WeaponRowName, TEXT("ComboDataContext")))
	{
		ComboMontage = ComboData->LightAttackMontage.Get();
		MaxComboCount = ComboData->MaxLightComboCount;
	}
	
	if (!ComboMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	CurrentComboIndex = 1;
	bIsComboQueued = false;
	bIsInputBufferOpen = false;
	CurrentMontageTask = nullptr;
	
	UAbilityTask_WaitGameplayEvent* HitTask = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Hit);
	HitTask->EventReceived.AddDynamic(this, &ThisClass::OnHitEventReceived);
	HitTask->ReadyForActivation();
	
	// 수정 4: 하드 코딩 제거 
	FGameplayTag InputEnableTag = KOGameplayTags::Event_Combo_EnableInput; 
	UAbilityTask_WaitGameplayEvent* InputEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, InputEnableTag);
	if (InputEventTask)
	{
		InputEventTask->EventReceived.AddDynamic(this, &ThisClass::UKOGA_Attack_Light::OnInputBufferOpened);
		InputEventTask->ReadyForActivation();
	}
	
	// 수정 5: 하드 코딩 제거 
	FGameplayTag ComboCheckTag = KOGameplayTags::Event_Combo_Check;
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

	if (bIsInputBufferOpen && CurrentComboIndex < MaxComboCount)
	{
		bIsComboQueued = true;
		UE_LOG(LogTemp, Warning, TEXT("bIsComboQueued true"));
	}
}

void UKOGA_Attack_Light::PlayNextComboSection()
{
	// 몽타주 1 섹션 -> 바인드 해제 다시 / 몽타주 2 섹션 -> 바인드 해제 / 다시 
	// 바인드 -> 몽타주 재생 -> 섹션 2 -> 섹션 3? 
	
	
	
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

void UKOGA_Attack_Light::OnInputBufferOpened(FGameplayEventData Payload)
{
	bIsInputBufferOpen = true;
}


