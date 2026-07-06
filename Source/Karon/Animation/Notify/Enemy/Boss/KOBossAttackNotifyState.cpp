#include "KOBossAttackNotifyState.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Data/Character/Enemy/KOEnemyDebugUserSettings.h"

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
 
	//에디터 개인설정(Editor Preferences > Karon > Enemy Debug)에서 일괄 컨트롤
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

		// ─── 수정 : Event_SkillHit 전송 제거 ─────────────────
		// GA 의존 없이 노티파이스테이트에서 직접 데미지 적용
		// 보스 ASC의 AttackPower × AttackCoefficient로 계산
		ApplyDamageToTarget(ASC, TargetActor);

		// Event_HitReact는 타겟 ASC로 전송 (히트스톱 등 반응용)
		UAbilitySystemComponent* TargetASC =
			TargetASCInterface->GetAbilitySystemComponent();
		FGameplayEventData HitReactData;
		HitReactData.Instigator = Owner;
		HitReactData.Target = TargetActor;
		TargetASC->HandleGameplayEvent(KOGameplayTags::Event_HitReact, &HitReactData);
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

// ─── 추가 : 보스 AttackPower × AttackCoefficient로 데미지 적용 ──
void UKOBossAttackNotifyState::ApplyDamageToTarget(
	UAbilitySystemComponent* OwnerASC,
	AActor* TargetActor)
{
	if (!OwnerASC || !TargetActor) return;

	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return;

	// 보스 CombatSet에서 AttackPower 읽기
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
