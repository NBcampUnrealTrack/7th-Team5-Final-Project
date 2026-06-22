#include "KOGA_AttackBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Tag/Data/KOGameplayTags_Data.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "Character/KOCharacterBase.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"


UKOGA_AttackBase::UKOGA_AttackBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UKOGA_AttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UAbilityTask_WaitGameplayEvent* TraceStartTask = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Trace_Start);
	TraceStartTask->EventReceived.AddDynamic(this, &ThisClass::OnWeaponTraceStarted);
	TraceStartTask->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* TraceEndTask = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Trace_End);
	TraceEndTask->EventReceived.AddDynamic(this, &ThisClass::OnWeaponTraceEnded);
	TraceEndTask->ReadyForActivation();
}

void UKOGA_AttackBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	GetWorld()->GetTimerManager().ClearTimer(TraceTimerHandle);
	ClearHitHistory();
}

void UKOGA_AttackBase::SendAttackEventsToTarget(FGameplayEventData* InEventData)
{
	if (!InEventData || !InEventData->Target) return;
	const UObject* RawdTarget = InEventData->Target;
	
	AActor* TargetActor = Cast<AActor>(const_cast<UObject*>(RawdTarget));
	if (!TargetActor) return;
	
	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	
	for (auto EventTag : AttackEventTags)
	{
		FGameplayEventData EventData;
		EventData.Instigator = Cast<const AActor>(GetAvatarCharacter());
		EventData.Target = TargetActor; 
		
		TargetASC->HandleGameplayEvent(EventTag, &EventData);
	}
}

void UKOGA_AttackBase::SendAttackEventsToTarget(AActor* TargetActor)
{
	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	
	for (auto EventTag : AttackEventTags)
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

void UKOGA_AttackBase::PerformWeaponTrace()
{
	ACharacter* Avatar = Cast<ACharacter>(GetAvatarCharacter());
	if (!Avatar) return;
	
	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	Avatar->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);

	USkeletalMeshComponent* TargetMesh = nullptr;

	for (USkeletalMeshComponent* Comp : SkeletalMeshes)
	{
		if (Comp->DoesSocketExist(WeaponStartSocket))
		{
			TargetMesh = Comp;
			break;
		}
	}

	if (!TargetMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s]  %s 소켓 찾을 수 없음."), *GetName(), *WeaponStartSocket.ToString());
		return;
	}
	
	FVector StartLoc = TargetMesh->GetSocketLocation(WeaponStartSocket);
	FVector EndLoc = TargetMesh->GetSocketLocation(WeaponEndSocket);
	
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Avatar);
	
	TArray<AActor*> AttachedActors;
	Avatar->GetAttachedActors(AttachedActors);
	ActorsToIgnore.Append(AttachedActors);
	
	FHitResult HitResult;
	EDrawDebugTrace::Type DebugType = bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	
	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		this,
		StartLoc,
		EndLoc,
		TraceRadius,
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
		
		if (!DamagedActors.Contains(HitActor))
		{
			DamagedActors.Add(HitActor);
			
			SendAttackEventsToTarget(HitActor);
			ApplyHitEffects(HitActor);
		}
	}
}

void UKOGA_AttackBase::ClearHitHistory()
{
	DamagedActors.Empty();
}

void UKOGA_AttackBase::OnWeaponTraceStarted(FGameplayEventData Payload)
{
	GetWorld()->GetTimerManager().SetTimer(
		TraceTimerHandle, 
		this,
		&UKOGA_AttackBase::PerformWeaponTrace, 
		0.016f, 
		true
	);
}

void UKOGA_AttackBase::OnWeaponTraceEnded(FGameplayEventData Payload)
{
	GetWorld()->GetTimerManager().ClearTimer(TraceTimerHandle);
	ClearHitHistory();
}

