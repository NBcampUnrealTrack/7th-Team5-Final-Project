#include "KOGA_AttackBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Ability/AbilityTask/AbilityTask_Tick.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/KOCharacterBase.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

UKOGA_AttackBase::UKOGA_AttackBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	AttackEventTags.AddTag(KOGameplayTags::Event_HitReact); 
}

void UKOGA_AttackBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (TickTask)
	{
		TickTask->StopTask(); 
		TickTask = nullptr;
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_AttackBase::SendAttackEventsToTarget(FGameplayEventData* InEventData)
{
	if (!InEventData || !InEventData->Target) return;
	const UObject* RawTarget = InEventData->Target;
	
	AActor* TargetActor = Cast<AActor>(const_cast<UObject*>(RawTarget));
	if (!TargetActor) return;
	
	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return;
	
	for (const auto& EventTag : AttackEventTags)
	{
		FGameplayEventData EventData;
		EventData.Instigator = Cast<const AActor>(GetAvatarCharacter());
		EventData.Target = TargetActor; 
		
		TargetASC->HandleGameplayEvent(EventTag, &EventData);
	}
}

void UKOGA_AttackBase::SendAttackEventsToTarget(AActor* TargetActor)
{
	if (!TargetActor) return; 
	
	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return;
	
	for (const auto& EventTag : AttackEventTags)
	{
		FGameplayEventData EventData;
		EventData.Instigator = Cast<const AActor>(GetAvatarCharacter());
		EventData.Target = TargetActor; 
		
		TargetASC->HandleGameplayEvent(EventTag, &EventData);
	}
}

void UKOGA_AttackBase::ApplyHitEffects(FGameplayEventData* InEventData)
{
	if (!InEventData || !InEventData->Target) return;
	
	const UObject* RawTarget = InEventData->Target;
	AActor* TargetActor = Cast<AActor>(const_cast<UObject*>(RawTarget));
	if (!TargetActor) return;
	
	UAbilitySystemComponent* SourceASC = GetASC(); 
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!SourceASC || !TargetASC) return;
	
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(GetAvatarCharacter());
	
	for (const FKOHitEffectData& Effect : HitAppliedEffects)
	{
		FGameplayEffectSpecHandle SpecHandle = 
		   SourceASC->MakeOutgoingSpec(Effect.EffectClass, Effect.Level, Context);
		if (!SpecHandle.IsValid()) continue;
		
		for (const auto& Pair : Effect.SetByCallerValues)
		{
			SpecHandle.Data->SetSetByCallerMagnitude(Pair.Key, Pair.Value);
		}
		
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}

void UKOGA_AttackBase::ApplyHitEffects(AActor* TargetActor)
{
	if (!TargetActor) return; 
	
	UAbilitySystemComponent* SourceASC = GetASC(); 
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!SourceASC || !TargetASC) return;
	
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(GetAvatarCharacter());
	
	for (const FKOHitEffectData& Effect : HitAppliedEffects)
	{
		FGameplayEffectSpecHandle SpecHandle = 
		   SourceASC->MakeOutgoingSpec(Effect.EffectClass, Effect.Level, Context);
		if (!SpecHandle.IsValid()) continue;
		
		for (const auto& Pair : Effect.SetByCallerValues)
		{
			SpecHandle.Data->SetSetByCallerMagnitude(Pair.Key, Pair.Value);
		}
		
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}

UKOCombatSet* UKOGA_AttackBase::GetCombatSet()
{
	AKOCharacterBase* Character = Cast<AKOCharacterBase>(GetAvatarCharacter());
	if (!Character) return nullptr;
	
	return Character->GetCombatSet(); 
}

void UKOGA_AttackBase::PerformWeaponTrace(float DeltaTime)
{
	// 1. Character를 Character로 캐스팅 
	ACharacter* Avatar =GetAvatarCharacter();
	if (!Avatar) return;
	
	// 2. 무기는 Skeletal이 아님 액터-> Static Mesh 
	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	Avatar->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);

	USkeletalMeshComponent* TargetMesh = nullptr;

	for (USkeletalMeshComponent* Comp : SkeletalMeshes)
	{
		if (Comp->DoesSocketExist(TraceData.StartSocket))
		{
			TargetMesh = Comp;
			break;
		}
	}

	if (!TargetMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s]  %s 소켓 찾을 수 없음."), *GetName(), *TraceData.StartSocket.ToString());
		return;
	}
	
	FVector StartLoc = TargetMesh->GetSocketLocation(TraceData.StartSocket);
	FVector EndLoc = TargetMesh->GetSocketLocation(TraceData.EndSocket);
	
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Avatar);
	
	TArray<AActor*> AttachedActors;
	Avatar->GetAttachedActors(AttachedActors);
	ActorsToIgnore.Append(AttachedActors);
	
	FHitResult HitResult;
	EDrawDebugTrace::Type DebugType = TraceData.bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	
	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		this,
		StartLoc,
		EndLoc,
		TraceData.TraceRadius,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false,
		ActorsToIgnore,
		DebugType,
		HitResult,
		true
	);

	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();
		
		if (!TraceData.HitActors.Contains(HitActor))
		{
			TraceData.HitActors.Add(HitActor);
			
			SendAttackEventsToTarget(HitActor);
			ApplyHitEffects(HitActor);
		}
	}
}

void UKOGA_AttackBase::ResetHitActors()
{
	TraceData.HitActors.Empty();
}
