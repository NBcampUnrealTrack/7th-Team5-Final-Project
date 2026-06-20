#include "AbilitySystem/Ability/Enemy/Boss/Attack/KOGA_BossAOEAttackBase.h"
#include "AIController.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Boss/KOAIC_BossChapter01.h"
#include "Engine/OverlapResult.h"

UKOGA_BossAOEAttackBase::UKOGA_BossAOEAttackBase()
{
}
 
void UKOGA_BossAOEAttackBase::ActivateAbility(
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
	
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		KOGameplayTags::Event_Boss01_Shockwave,
		nullptr,
		true,
		true
	);
	
	WaitEventTask->EventReceived.AddDynamic(this, &UKOGA_BossAOEAttackBase::OnShockwaveNotify);
	WaitEventTask->ReadyForActivation();
}
 
void UKOGA_BossAOEAttackBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// 타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ShockwaveTimerHandle);
	}
 
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_BossAOEAttackBase::OnMontageCompleted()
{
	// 충격파가 아직 진행 중이면 EndAbility 호출 안 함
	if (CurrentShockwaveCount < ShockwaveCount)
	{
		return;
	}
	
	Super::OnMontageCompleted();
}

void UKOGA_BossAOEAttackBase::OnShockwaveNotify(FGameplayEventData EventData)
{
	CurrentShockwaveCount = 0;
 
	GetWorld()->GetTimerManager().SetTimer(
		ShockwaveTimerHandle,
		this,
		&UKOGA_BossAOEAttackBase::TriggerShockwave,
		ShockwaveInterval,
		true,
		0.f
	);
}
 
void UKOGA_BossAOEAttackBase::TriggerShockwave()
{
	ACharacter* Character = GetAvatarCharacter();
	if (!Character) return; 
 
	// 디버그 원형 표시
	if (bShowDebug)
	{
		DrawDebugCircle(
			GetWorld(),
			Character->GetActorLocation(),
			ShockwaveRadius,
			32,
			FColor::Red,
			false,
			0.5f,
			0,
			3.f,
			FVector(1, 0, 0),
			FVector(0, 1, 0)
		);
	}
 
	// 범위 내 액터 감지
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(ShockwaveRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);
 
	GetWorld()->OverlapMultiByChannel(
		Overlaps,
		Character->GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		Sphere,
		QueryParams
	);
 
	for (const FOverlapResult& Overlap : Overlaps)
	{
		ACharacter* Target = Cast<ACharacter>(Overlap.GetActor());
		if (!Target) continue;
		
		// 공중이면 피격 무시 (점프 회피)
		if (!Target->GetCharacterMovement()->IsMovingOnGround()) continue;
		
		// 직접 데미지 적용
		SendAttackEventsToTarget(Target); 
		ApplyHitEffects(Target);
	}
	
	CurrentShockwaveCount++;
	
	if (CurrentShockwaveCount >= ShockwaveCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(ShockwaveTimerHandle);
		
		AAIController* AIC = Cast<AAIController>(Character->GetController());
		if (AIC)
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				BB->SetValueAsBool(AKOAIC_BossChapter01::bIsGroggyKey, true);
			}
		}
		
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
