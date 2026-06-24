#include "KOGA_Movement_Sprint.h"

#include "Abilities/Tasks/AbilityTask_WaitAttributeChange.h"
#include "AbilitySystem/Attribute/KOStaminaSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/Hero/KOHeroCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UKOGA_Movement_Sprint::UKOGA_Movement_Sprint()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Movement_Sprint));
}

bool UKOGA_Movement_Sprint::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) return false;
	
	ACharacter* Character = GetAvatarCharacter();
	if (!Character) return false;
	
	UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement(); 
	if (!CharacterMovement) return false;
	
	// 떨어지는 경우 
	if (CharacterMovement->IsFalling()) return false; 
	
	// 스테미나가 없는 경우 
	const float Stamina = GetASC()->GetNumericAttribute(UKOStaminaSet::GetStaminaAttribute());
	if (Stamina <= 0.f) return false;
	
	// 움직이지 않는 경우 
	const FVector Velocity = CharacterMovement->Velocity;
	const float CurrentSpeed = Velocity.Size2D();
	if (CurrentSpeed <= 30.f) return false;
	
	return true; 
}

void UKOGA_Movement_Sprint::ActivateAbility(
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
	
	// 1. Info 확인 
	CachedCharacter = Cast<AKOHeroCharacter>(GetAvatarCharacter()); 
	if (!CachedCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
		
	CachedMovement = CachedCharacter->GetCharacterMovement(); 
	if (!CachedMovement)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// 2. Effect 적용 
	if (SprintEffect)
	{
		SprintEffectHandle = ApplyEffectToSelf(SprintEffect);
	}
	
	CachedCharacter->UpdateGait(EGait::Sprint);
	
	// 3. Stamina 감소시 마다 달리기 조건 체크 Task 
	UAbilityTask_WaitAttributeChange* CheckStaminaTask =
		UAbilityTask_WaitAttributeChange::WaitForAttributeChange(
			this,
			UKOStaminaSet::GetStaminaAttribute(),
			FGameplayTag::EmptyTag,
			FGameplayTag::EmptyTag,
			false
		);
	
	if (CheckStaminaTask)
	{
		CheckStaminaTask->OnChange.AddDynamic(this, &ThisClass::OnStaminaChanged);
		CheckStaminaTask->ReadyForActivation(); 
	}
}

void UKOGA_Movement_Sprint::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	GetWorld()->GetTimerManager().ClearTimer(GraceTimer);
	
	if (SprintEffectHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(SprintEffectHandle);
	}
	
	if (CachedCharacter)
	{
		CachedCharacter->UpdateGait(EGait::Run); 
		CachedCharacter = nullptr;
	}
	
	CachedMovement = nullptr;
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_Movement_Sprint::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	
	if (IsActive())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UKOGA_Movement_Sprint::OnStaminaChanged()
{
	if (!CachedCharacter || !CachedMovement) return; 
	
	// 스테미나가 체크 
	const float CurrentStamina = GetASC()->GetNumericAttribute(UKOStaminaSet::GetStaminaAttribute());
	if (CurrentStamina <= 0.01f)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	// 캐릭터가 멈추거나 떨어지는 경우 : 유예 
	if (CachedMovement->GetCurrentAcceleration().IsNearlyZero() || CachedMovement->IsFalling())
	{
		TryStartGraceTimer();
		return; 
	}
	
	GetWorld()->GetTimerManager().ClearTimer(GraceTimer);
}

void UKOGA_Movement_Sprint::TryStartGraceTimer()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(GraceTimer)) return;
	
	TWeakObjectPtr<UKOGA_Movement_Sprint> WeakThis(this);
	GetWorld()->GetTimerManager().SetTimer(
		GraceTimer,
		[WeakThis]()
		{
			if (!WeakThis.IsValid() || !WeakThis->IsActive()) return;
			
			WeakThis->EndAbility(
				WeakThis->CurrentSpecHandle,
				WeakThis->CurrentActorInfo,
				WeakThis->CurrentActivationInfo,
				true, false
			);
		},
		GraceTime
		, false
	);
}

