#include "Character/Enemy/Boss/GA/KOGA_BossAOEAttackBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
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
	
	if (!IsActive())
	{
		return;
	}
 
	WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		KOGameplayTags::Event_Boss01_Shockwave,
		nullptr,
		true,
		true
	);
 
	if (WaitEventTask)
	{
		WaitEventTask->EventReceived.AddDynamic(
			this, &UKOGA_BossAOEAttackBase::OnShockwaveNotify
		);
		WaitEventTask->ReadyForActivation();
	}
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
 
	if (WaitEventTask)
	{
		WaitEventTask->EndTask();
		WaitEventTask = nullptr;
	}
 
	Super::EndAbility(Handle, ActorInfo, ActivationInfo,
		bReplicateEndAbility, bWasCancelled);
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
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return;
	}
 
	// 디버그 원형 표시
	if (bShowDebug)
	{
		DrawDebugCircle(
			GetWorld(),
			Avatar->GetActorLocation(),
			ShockwaveRadius,
			32,
			FColor::Red,
			false,
			ShockwaveInterval,
			0,
			2.f,
			FVector(1, 0, 0),
			FVector(0, 1, 0)
		);
	}
 
	// 범위 내 액터 감지
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(ShockwaveRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Avatar);
 
	GetWorld()->OverlapMultiByChannel(
		Overlaps,
		Avatar->GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		Sphere,
		QueryParams
	);
 
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor)
		{
			continue;
		}
 
		ACharacter* HitCharacter = Cast<ACharacter>(HitActor);
		if (!HitCharacter)
		{
			continue;
		}
 
		// 공중이면 피격 무시 (점프 회피)
		if (!HitCharacter->GetCharacterMovement()->IsMovingOnGround())
		{
			continue;
		}
 
		// 데미지 이벤트 발생
		FGameplayEventData EventData;
		EventData.Target = HitActor;
		GetAbilitySystemComponentFromActorInfo()->HandleGameplayEvent(
			KOGameplayTags::Event_Boss01_Shockwave,
			&EventData
		);
	}
	
	CurrentShockwaveCount++;
	
	if (CurrentShockwaveCount >= ShockwaveCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(ShockwaveTimerHandle);
		
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		
		// TODO: 그로기 진입 처리
		// 보스 액터에서 OnGroggyBegin 호출
	}
}
