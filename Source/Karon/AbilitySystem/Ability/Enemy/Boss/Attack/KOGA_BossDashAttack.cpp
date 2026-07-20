#include "AbilitySystem/Ability/Enemy/Boss/Attack/KOGA_BossDashAttack.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "AbilitySystem/Tag/State/KOGameplayTags_State.h"
#include "Character/Enemy/Boss/KOBossBase.h"
#include "Character/Enemy/Boss/Chapter01/Gimmick/KOBossCH01GimmickPillar.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Data/KO_HitData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UKOGA_BossDashAttack::UKOGA_BossDashAttack()
{
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Boss_Dashing);
	
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Boss_Attacking);
}
 
void UKOGA_BossDashAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UGameplayAbility::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
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
	
	AKOBossBase* Boss = Cast<AKOBossBase>(Character);
	AActor* Target = Boss ? Boss->CurrentTarget : nullptr;
 
	DashDirection = Target ?
		(Target->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal() :
		Character->GetActorForwardVector();
	DashDirection.Z = 0.f;
	
	DashedActors.Empty();
	
	if (PreDashMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, PreDashMontage, 1.f, NAME_None, false);

		MontageTask->OnCompleted.AddDynamic(this, &UKOGA_BossDashAttack::OnPreDashMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UKOGA_BossDashAttack::OnPreDashMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &UKOGA_BossDashAttack::OnPreDashMontageCancelled);
		MontageTask->ReadyForActivation();
	}
	else
	{
		// 전조 몽타주 없으면 즉시 돌진
		StartDash();
	}
}
 
void UKOGA_BossDashAttack::OnPreDashMontageCompleted()
{
	if (ACharacter* Character = GetAvatarCharacter())
	{
		DashDirection = Character->GetActorForwardVector();
		DashDirection.Z = 0.f;
		DashDirection.Normalize();
	}
 
	StartDash();
}
 
void UKOGA_BossDashAttack::OnPreDashMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
 
void UKOGA_BossDashAttack::StartDash()
{
	ACharacter* Character = GetAvatarCharacter();
	if (!Character)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	if (AKOBossBase* Boss = Cast<AKOBossBase>(Character))
	{
		Boss->OnDashSmokeBegin();
	}
 
	Character->GetCapsuleComponent()->OnComponentHit.AddDynamic(
		this, &UKOGA_BossDashAttack::OnDashHit
	);
 
	// 최대 돌진 시간 타이머
	GetWorld()->GetTimerManager().SetTimer(
		DashTimerHandle,
		this,
		&UKOGA_BossDashAttack::OnDashTimeOut,
		DashDuration,
		false
	);
 
	GetWorld()->GetTimerManager().SetTimer(
		DashVelocityTimerHandle,
		FTimerDelegate::CreateLambda([this]()
		{
			ACharacter* Char = GetAvatarCharacter();
			if (!IsActive() || !Char) return;
			Char->GetCharacterMovement()->Velocity = DashDirection * DashSpeed;
		}),
		0.016f,
		true
	);
}
 
void UKOGA_BossDashAttack::OnDashHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!OtherActor)
	{
		return;
	}
	
	if (bIsGimmickDash && OtherActor->ActorHasTag(FName("Object_BossCH01_Gimmick_Pillar")))
	{
		HandleGimmickPillarHit(OtherActor); 
		return;
	}
	
	// 데미지 주고 계속 돌진
	IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(OtherActor);
	if (TargetASI && TargetASI->GetAbilitySystemComponent())
	{
		// 중복 방지
		bool bAlreadyHit = DashedActors.ContainsByPredicate(
			[OtherActor](const TWeakObjectPtr<AActor>& Weak)
			{
				return Weak.IsValid() && Weak.Get() == OtherActor;
			});

		if (!bAlreadyHit)
		{
			DashedActors.Add(OtherActor);
			ApplyHitEffects(OtherActor);

			UAbilitySystemComponent* TargetASC =
				TargetASI->GetAbilitySystemComponent();
			FGameplayEventData EventData;
			EventData.Instigator = GetAvatarCharacter();
			EventData.Target = OtherActor;
			if (HitData)
			{
				EventData.OptionalObject = HitData.Get();
			}

			TargetASC->HandleGameplayEvent(KOGameplayTags::Event_HitReact, &EventData);
		}
		return; // 돌진 계속
	}
	StopDash();
}
 
// 기믹 기둥 처리
void UKOGA_BossDashAttack::HandleGimmickPillarHit(AActor* PillarActor)
{
	if (AKOBossCH01GimmickPillar* Pillar = Cast<AKOBossCH01GimmickPillar>(PillarActor))
	{
		Pillar->BreakPillar();
	}
	
	AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarCharacter());
	if (Boss)
	{
		Boss->TriggerGroggy();
		
		if (GimmickSuccessSFX)
		{
			UGameplayStatics::PlaySoundAtLocation(
				GetWorld(),
				GimmickSuccessSFX,
				Boss->GetActorLocation()
			);
		}
	}
 
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayCueParameters Params;
		Params.EffectContext = ASC->MakeEffectContext();

		UE_LOG(LogTemp, Log, TEXT("[DashAttack] ExecuteGameplayCue 호출 : %s"),
			*KOGameplayTags::GameplayCue_CameraShake_BossAttack.GetTag().ToString());

		ASC->ExecuteGameplayCue(
			KOGameplayTags::GameplayCue_CameraShake_BossAttack,
			Params
		);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[DashAttack] ASC nullptr - GameplayCue 호출 실패"));
	}
	
	StopDash();
}
 
void UKOGA_BossDashAttack::StopDash()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		Character->GetCharacterMovement()->Velocity = FVector::ZeroVector;
		Character->GetCapsuleComponent()->OnComponentHit.RemoveAll(this);

		if (AKOBossBase* Boss = Cast<AKOBossBase>(Character))
		{
			Boss->OnDashSmokeEnd();
		}
	}
 
	GetWorld()->GetTimerManager().ClearTimer(DashTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(DashVelocityTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(DashHitScanTimerHandle);
	DashedActors.Empty();
 
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
 
// 최대 시간 초과 종료
void UKOGA_BossDashAttack::OnDashTimeOut()
{
	StopDash();
}
 
void UKOGA_BossDashAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DashTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(DashVelocityTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(DashHitScanTimerHandle);
	}
	DashedActors.Empty();
 
	ACharacter* Character = GetAvatarCharacter();
	if (Character)
	{
		Character->GetCapsuleComponent()->OnComponentHit.RemoveAll(this);
		Character->GetCharacterMovement()->Velocity = FVector::ZeroVector;
		
		if (bIsGimmickDash)
		{
			AKOBossBase* Boss = Cast<AKOBossBase>(Character);
			if (Boss)
			{
				Boss->NotifyGimmickDashEnd();
			}
		}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
