// Fill out your copyright notice in the Description page of Project Settings.


#include "KOGA_ParryAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "IMediaCache.h"
#include "MotionWarpingComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Ability/Status/KOGA_Parried.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "AbilitySystem/Tag/State/KOGameplayTags_State.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "Character/Hero/KOHeroCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UKOGA_ParryAttack::UKOGA_ParryAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = KOGameplayTags::Event_ParryAttack; 
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
	
	// 사망 / 히트 중 진입차단 
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Dead);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_HitReacting);
}

void UKOGA_ParryAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	//이벤트데이터에서 감지된 에너미 액터를 가져온다.
	if (TriggerEventData)
	{
		FGameplayAbilityTargetDataHandle TargetData = TriggerEventData->TargetData;

		TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetActorsFromTargetData(TargetData, 0);

		for (AActor* Target : TargetActors)
		{
			//첫번째로 감지된 몬스터만 타겟
			if (AKOBaseEnemy* Enemy = Cast<AKOBaseEnemy>(Target))
			{
				ExecuteParryAttack(Enemy);
				break;
			}
		}
	}
}

void UKOGA_ParryAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, 
	bool bReplicateEndAbility,bool bWasCancelled)
{
	// 플레이어 이동, 회전 입력 가능
	if (APlayerController* PC = Cast<APlayerController>(CachedPlayer->GetController()))
	{
		PC->SetIgnoreMoveInput(false);
	}
	CharacterRotateLock(false);

	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_ParryAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_ParryAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_ParryAttack::OnNotifyHitEvent(FGameplayEventData HitGameplayEventData)
{
	UAnimInstance* EnemyAnimInstance = CachedEnemy->GetMesh()->GetAnimInstance();
	// Early Return
	if (!EnemyAnimInstance || !CachedPlayer || !CachedEnemy)
	{
		return;
	}
	
	//에너미 몽타주 점프
	EnemyAnimInstance->Montage_JumpToSection(CachedParriedGA->ParriedSuccessSectionName, EnemyAnimInstance->GetCurrentActiveMontage());
	
	ApplyHitEffects(CachedEnemy);
}

void UKOGA_ParryAttack::ExecuteParryAttack(AKOBaseEnemy* Enemy)
{
	CachedEnemy=Enemy;
	CachedPlayer = Cast<AKOHeroCharacter>(GetAvatarCharacter());
	UAbilitySystemComponent* AbilitySystemComponent = Enemy->GetAbilitySystemComponent();
	UAnimInstance* EnemyAnimInstance = Enemy->GetMesh()->GetAnimInstance();
	// Early Return
	if (!EnemyAnimInstance || !CachedPlayer || !AbilitySystemComponent || !CachedEnemy)
	{
		return;
	}
	
	//Enemy의 ParriedGA를 가져온다.
	CachedParriedGA = Cast<UKOGA_Parried>(AbilitySystemComponent->GetAnimatingAbility());
	if (!CachedParriedGA)
	{
		return;
	}
	
	// 플레이어 이동, 회전 입력 방지
	if (APlayerController* PC = Cast<APlayerController>(CachedPlayer->GetController()))
	{
		PC->SetIgnoreMoveInput(false);
	}
	
	CharacterRotateLock(true);
	
	
	//플레이어를 바라보도록 몬스터 회전값을 수정
	FVector LookAtVector = CachedPlayer->GetActorLocation() - Enemy->GetActorLocation();
	LookAtVector.Z = 0.0f;
	if (!LookAtVector.IsNearlyZero())
	{
		FRotator NewRotation = LookAtVector.Rotation();
		Enemy->SetActorRotation(NewRotation);
	}
	
	//자동회복 방지
	CachedParriedGA->ClearRecoverTimer();

	//모션 워핑
	UMotionWarpingComponent* MotionWarpingComponent = CachedPlayer->FindComponentByClass<UMotionWarpingComponent>();
	if (MotionWarpingComponent)
	{
		// 에너미 정면에서 해당 거리만큼 놓인 곳으로 이동
		FVector EnemyLocation = CachedEnemy->GetActorLocation();
		FVector EnemyForward = CachedEnemy->GetActorForwardVector();
		FVector WarpLocation = EnemyLocation + (EnemyForward * MotionWarpingDistance); 
		// 높이는 플레이어 자신의 높이를 유지해 땅에 묻히는 것을 방지
		WarpLocation.Z = CachedPlayer->GetActorLocation().Z;

		// 몬스터를 바라보도록 설정
		FRotator WarpRotation = (-EnemyForward).Rotation();
		WarpRotation.Pitch = 0.0f;
		WarpRotation.Roll = 0.0f;
		
		// 모션워핑 컴포넌트에 등록
		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
			MotionWarpingName, 
			WarpLocation, 
			WarpRotation
		);
	}
	
	
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
	this, KOGameplayTags::Event_Hit, nullptr, false, false);

	WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnNotifyHitEvent);
	WaitEventTask->ReadyForActivation();
	
	
	//플레이어 해당하는 몽타주 실행
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			Montage,
			1.0f,
			NAME_None,
			false
		);
	
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UKOGA_ParryAttack::CharacterRotateLock(bool bIsLocked)
{
	// 플레이어 컨트롤러의 기본 회전 동기화 끄기
	CachedPlayer->bUseControllerRotationYaw = !bIsLocked;

	if (UCharacterMovementComponent* MoveComp = CachedPlayer->GetCharacterMovement())
	{
		// 이동 방향으로 캐릭터가 자동으로 도는 옵션 끄기
		MoveComp->bOrientRotationToMovement = !bIsLocked; 
    
		// 컨트롤러가 바라보는 방향으로 자동으로 도는 옵션 끄기
		MoveComp->bUseControllerDesiredRotation = !bIsLocked; 
	}

	// 마우스 입력으로 인한 컨트롤러 회전 자체를 완전히 잠그기
	if (APlayerController* PC = Cast<APlayerController>(CachedPlayer->GetController()))
	{
		PC->SetIgnoreLookInput(bIsLocked); 
	}
}
