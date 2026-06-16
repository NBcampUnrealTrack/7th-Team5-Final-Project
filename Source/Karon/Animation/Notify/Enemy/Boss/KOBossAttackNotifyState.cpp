#include "KOBossAttackNotifyState.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystem/Ability/Enemy/Boss/Attack/KOGA_BossMeleeAttackBase.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"

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
 
	// 소켓 존재 여부 확인
	if (!MeshComp->DoesSocketExist(AttackSocketName))
	{
		UE_LOG(LogTemp, Warning,TEXT("[BossMeleeNotify] 소켓 없음 : %s"), *AttackSocketName.ToString());
		return;
	}
 
	PrevSocketLocation = MeshComp->GetSocketLocation(AttackSocketName);
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
	
	if (!MeshComp->DoesSocketExist(AttackSocketName))
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
 
	const FVector CurrSocketLocation = MeshComp->GetSocketLocation(AttackSocketName);
	
	TArray<FHitResult> HitResults;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Owner);
 
	EDrawDebugTrace::Type DebugType = bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
 
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
 
	PrevSocketLocation = CurrSocketLocation;
 
	if (!bHit)
	{
		return;
	}
 
	TArray<AActor*> ActorsToHit;
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HittedActor = HitResult.GetActor();
		if (!HittedActor)
		{
			continue;
		}
 
		bool bAlreadyHit = false;
		for (const TWeakObjectPtr<AActor>& WeakActor : HittedActors)
		{
			if (WeakActor.IsValid() && WeakActor.Get() == HittedActor)
			{
				bAlreadyHit = true;
				break;
			}
		}
 
		if (!bAlreadyHit)
		{
			HittedActors.Add(HittedActor);
			ActorsToHit.Add(HittedActor);
		}
	}
 
	for (AActor* TargetActor : ActorsToHit)
	{
		FGameplayEventData HitGameplayEventData;
		HitGameplayEventData.Target = TargetActor;
		ASC->HandleGameplayEvent(KOGameplayTags::Event_SkillHit,&HitGameplayEventData);
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
