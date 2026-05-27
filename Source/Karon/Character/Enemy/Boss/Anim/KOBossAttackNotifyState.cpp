#include "Character/Enemy/Boss/Anim/KOBossAttackNotifyState.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/Enemy/Boss/GA/KOGA_BossMeleeAttackBase.h"
#include "Kismet/KismetSystemLibrary.h"
 
void UKOBossAttackNotifyState::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}
 
	// 현재 활성화된 GA에서 소켓 위치 가져오기
	AActor* Owner = MeshComp->GetOwner();
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Owner);
	if (!ASCInterface)
	{
		return;
	}
 
	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}
	
	UKOGA_BossMeleeAttackBase* CurrentGA = nullptr;
	
	const TArray<FGameplayAbilitySpec>& AbilitySpecs = ASC->GetActivatableAbilities();
	
	for (const FGameplayAbilitySpec& Spec : AbilitySpecs)
	{
		if (!Spec.IsActive())
		{
			continue;
		}

		CurrentGA = Cast<UKOGA_BossMeleeAttackBase>(Spec.Ability);
		if (CurrentGA)
		{
			break;
		}
	}
 
	if (!CurrentGA)
	{
		return;
	}
 
	// 소켓 위치로 초기화
	PrevSocketLocation = MeshComp->GetSocketLocation(CurrentGA->AttackSocketName);
	CurrSocketLocation = PrevSocketLocation;
	HittedActors.Empty();
}
 
void UKOBossAttackNotifyState::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
 
	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}
 
	AActor* Owner = MeshComp->GetOwner();
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Owner);
	if (!ASCInterface)
	{
		return;
	}
 
	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}
 
	// 현재 GA 찾기
	UKOGA_BossMeleeAttackBase* CurrentGA = nullptr;

	const TArray<FGameplayAbilitySpec>& AbilitySpecs = ASC->GetActivatableAbilities();
	for (const FGameplayAbilitySpec& Spec : AbilitySpecs)
	{
		if (!Spec.IsActive())
		{
			continue;
		}

		CurrentGA = Cast<UKOGA_BossMeleeAttackBase>(Spec.Ability);
		if (CurrentGA)
		{
			break;
		}
	}
 
	if (!CurrentGA)
	{
		return;
	}
 
	// 현재 소켓 위치 갱신
	PrevSocketLocation = MeshComp->GetSocketLocation(CurrentGA->AttackSocketName);
	
	TArray<FHitResult> HitResults;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Owner);
 
	EDrawDebugTrace::Type DebugType = bShowDebug ?
		EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
 
	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		Owner->GetWorld(),
		PrevSocketLocation,
		CurrSocketLocation,
		TraceRadius,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false,
		ActorsToIgnore,
		DebugType,
		HitResults,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		2.0f
	);
 
	// 이전 위치 갱신
	PrevSocketLocation = CurrSocketLocation;
 
	if (!bHit)
	{
		return;
	}
 
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HittedActor = HitResult.GetActor();
		if (!HittedActor || HittedActors.Contains(HittedActor))
		{
			continue;
		}
 
		HittedActors.Add(HittedActor);
 
		FGameplayEventData HitGameplayEventData;
		HitGameplayEventData.Target = HittedActor;
		ASC->HandleGameplayEvent(
			KOGameplayTags::Event_SkillHit,
			&HitGameplayEventData
		);
	}
}
 
void UKOBossAttackNotifyState::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
 
	HittedActors.Empty();
}
