#include "AbilitySystem/Ability/Enemy/Boss/Attack/KOGA_BossDashAttack.h"

#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "AbilitySystem/Tag/Object/KOGameplayTags_Object.h"
#include "AbilitySystem/Tag/State/KOGameplayTags_State.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Boss/KOAIC_BossChapter01.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UKOGA_BossDashAttack::UKOGA_BossDashAttack()
{
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Boss_Attacking);
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
	
	AAIController* AIC = Cast<AAIController>(Character->GetController());
	if (!AIC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
 
	UBlackboardComponent* BB = AIC->GetBlackboardComponent();
	if (!BB)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
 
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(AKOAIC_BossChapter01::TargetActorKey));
	DashDirection = Target ? 
		(Target->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal() :
		Character->GetActorForwardVector();
	DashDirection.Z = 0.f;
	
 
	// 충돌 이벤트 바인딩
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
	if (!OtherActor) return;
 
	// 기믹 돌진 기둥 태그 확인 
	if (bIsGimmickDash && OtherActor->ActorHasTag(FName("Object_BossCH01_Gimmick_Pillar")))
	{
		HandleGimmickPillarHit(OtherActor); 
		return;
	}
 
	ApplyHitEffects(OtherActor);
 
	// 일반 벽 충돌
	StopDash();
}
 
// 기믹 기둥 처리
void UKOGA_BossDashAttack::HandleGimmickPillarHit(AActor* PillarActor)
{
	if (PillarActor)
	{
		PillarActor->Destroy();
	}
	
	APawn* Pawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (Pawn)
	{
		AAIController* AIC = Cast<AAIController>(Pawn->GetController());
		if (AIC)
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				BB->SetValueAsBool(AKOAIC_BossChapter01::bIsGroggyKey, true);
			}
		}
	}
 
	StopDash();
}
 
// 돌진 종료
void UKOGA_BossDashAttack::StopDash()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		Character->GetCharacterMovement()->Velocity = FVector::ZeroVector;
		Character->GetCapsuleComponent()->OnComponentHit.RemoveAll(this);
	}
 
	GetWorld()->GetTimerManager().ClearTimer(DashTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(DashVelocityTimerHandle);
 
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
	}
 
	ACharacter* Character = GetAvatarCharacter();
	if (Character)
	{
		Character->GetCapsuleComponent()->OnComponentHit.RemoveAll(this);
		Character->GetCharacterMovement()->Velocity = FVector::ZeroVector;
		
		// 기믹 돌진시 BBkey값 변경
		if (bIsGimmickDash)
		{
			AAIController* AIC = Cast<AAIController>(Character->GetController());
			if (AIC)
			{
				if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
				{
					BB->SetValueAsBool(AKOAIC_BossChapter01::bIsGimmickReadyKey, false);
				}
			}
		}
	}
	
 
	Super::EndAbility(Handle, ActorInfo, ActivationInfo,bReplicateEndAbility, bWasCancelled);
}
