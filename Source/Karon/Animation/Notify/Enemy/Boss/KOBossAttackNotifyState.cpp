#include "KOBossAttackNotifyState.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Data/Character/Enemy/KOEnemyDebugUserSettings.h"

#include "Data/KO_HitData.h"
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
		return;
	}
 
	PrevSocketLocation = MeshComp->GetSocketLocation(AttackSocketName);
	HittedActors.Empty();
	bHitDetected = false;
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
 
	// ─── 추가 : 이미 피격됐으면 트레이스 스킵 ───────────────
	if (bHitDetected)
	{
		// 디버그는 계속 출력
		const bool bShowDebug = GetDefault<UKOEnemyDebugUserSettings>()->bShowAttackTraceDebug;
		if (bShowDebug)
		{
			UKismetSystemLibrary::DrawDebugSphere(
				Owner->GetWorld(),
				CurrSocketLocation,
				TraceRadius,
				12,
				FLinearColor::Gray,
				2.0f
			);
		}
 
		PrevSocketLocation = CurrSocketLocation;
		return;
	}
	
	TArray<FHitResult> HitResults;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Owner);
	
	const bool bShowDebug = GetDefault<UKOEnemyDebugUserSettings>()->bShowAttackTraceDebug;
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
 
	// 식별 액터 필터링 및 데미지 적용
	for (AActor* TargetActor : ActorsToHit)
	{
		IAbilitySystemInterface* TargetASCInterface = Cast<IAbilitySystemInterface>(TargetActor);
		if (!TargetASCInterface || !TargetASCInterface->GetAbilitySystemComponent())
		{
			continue;
		}
		
		ApplyDamageToTarget(ASC, TargetActor);
		
		UAbilitySystemComponent* TargetASC =
			TargetASCInterface->GetAbilitySystemComponent();
		FGameplayEventData HitReactData;
		HitReactData.Instigator = Owner;
		HitReactData.Target = TargetActor;
		
		if (HitData)
		{
			HitReactData.OptionalObject = HitData.Get();
		}
		
		TargetASC->HandleGameplayEvent(KOGameplayTags::Event_HitReact, &HitReactData);

		// ─── 추가 : 피격 성공 → 이후 트레이스 중단 ──────────
		bHitDetected = true;
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

void UKOBossAttackNotifyState::ApplyDamageToTarget(
	UAbilitySystemComponent* OwnerASC,
	AActor* TargetActor)
{
	if (!OwnerASC || !TargetActor) return;

	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return;
	
	const UKOCombatSet* CombatSet = OwnerASC->GetSet<UKOCombatSet>();
	const float AttackPower = CombatSet ? CombatSet->GetAttackPower() : 1.f;

	FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();

	for (const FKOBossAttackEffectData& Effect : DamageEffects)
	{
		if (!Effect.EffectClass) continue;

		FGameplayEffectSpecHandle Spec =
			OwnerASC->MakeOutgoingSpec(Effect.EffectClass, Effect.Level, Context);
		if (!Spec.IsValid()) continue;

		Spec.Data->SetSetByCallerMagnitude(
			KOGameplayTags::Data_AttackCoefficient,
			AttackPower * Effect.AttackCoefficient);

		OwnerASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
	}
}
 
