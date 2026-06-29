#include "KOGA_AttackBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Ability/AbilityTask/AbilityTask_Tick.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/KOCharacterBase.h"
#include "Component/Inventory/KOEquipmentComponent.h"
#include "GameFramework/Character.h"
#include "Items/Equipment/KOWeaponBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Utility/Log/KOLogManager.h"

UKOGA_AttackBase::UKOGA_AttackBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	AttackEventTags.AddTag(KOGameplayTags::Event_HitReact); 
	
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Character_Attacking);
}

void UKOGA_AttackBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	TraceData.TraceMesh = FindTraceMesh();
    
	if (!TraceData.TraceMesh)
	{
		KO_LOG(Combat, Error, TEXT("TraceMesh 없음"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	TraceData.bIsFirstTick = true;
	TraceData.HitActors.Empty();
	ApplySelfEffects();
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
	
	if (UAbilitySystemComponent* SourceASC = GetASC())
	{
		for (auto EffectHandle:SelfEffectsHandles)
		{
			SourceASC->RemoveActiveGameplayEffect(EffectHandle);
		}
		SelfEffectsHandles.Empty();
	}

	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_AttackBase::SendAttackEventsToTarget(FGameplayEventData* InEventData)
{
	if (!InEventData || !InEventData->Target) return;
	
	const UObject* RawTarget = InEventData->Target;
	if (AActor* TargetActor = Cast<AActor>(const_cast<UObject*>(RawTarget)))
	{
		SendAttackEventsToTarget(TargetActor);
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
	if (AActor* TargetActor = Cast<AActor>(const_cast<UObject*>(RawTarget)))
	{
		ApplyHitEffects(TargetActor);
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
	
	float AttackValue = GetCombatSet() ? GetCombatSet()->GetAttackPower() : 1.f;
	for (const FKOHitEffectData& Effect : DamageEffects)
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
	
	for (const FKOHitEffectData& Effect : AdditionalEffects)
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

void UKOGA_AttackBase::ApplySelfEffects()
{
	UAbilitySystemComponent* SourceASC = GetASC(); 
	if (!SourceASC) return;
	
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(GetAvatarCharacter());
	
	for (const FKOHitEffectData& Effect : SelfEffects)
	{
		FGameplayEffectSpecHandle SpecHandle = 
		   SourceASC->MakeOutgoingSpec(Effect.EffectClass, Effect.Level, Context);
		if (!SpecHandle.IsValid()) continue;
		
		for (const auto& Pair : Effect.SetByCallerValues)
		{
			SpecHandle.Data->SetSetByCallerMagnitude(Pair.Key, Pair.Value); 
		}
		FActiveGameplayEffectHandle Handle=SourceASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		if (Handle.IsValid())
		{
			SelfEffectsHandles.Add(Handle);
		}
	}
}

UKOCombatSet* UKOGA_AttackBase::GetCombatSet()
{
	AKOCharacterBase* Character = Cast<AKOCharacterBase>(GetAvatarCharacter());
	return Character ? Character->GetCombatSet() : nullptr; 
}

void UKOGA_AttackBase::PerformWeaponTrace(float DeltaTime)
{
	ACharacter* Avatar = GetAvatarCharacter();
	if (!Avatar || !TraceData.TraceMesh) return;
	
	if (TraceData.MaxHitCount > 0 && TraceData.HitActors.Num() >= TraceData.MaxHitCount) return;
	
	FVector CurrentStart = TraceData.TraceMesh->GetSocketLocation(TraceData.GetStartSocket());
	FVector CurrentEnd = TraceData.TraceMesh->GetSocketLocation(TraceData.GetEndSocket());
	
	if (TraceData.bIsFirstTick)
	{
		TraceData.PrevStartLocation = CurrentStart;
		TraceData.PrevEndLocation = CurrentEnd;
		TraceData.bIsFirstTick = false;
		return; 
	}
	
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Avatar);
	
	TArray<AActor*> AttachedActors;
	Avatar->GetAttachedActors(AttachedActors);
	ActorsToIgnore.Append(AttachedActors);
	
	EDrawDebugTrace::Type DebugType = TraceData.bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_Pawn);
	
	TArray<FHitResult> CombinedHits;
	
	TArray<FHitResult> StartHits;
	UKismetSystemLibrary::SphereTraceMulti(
		this, 
		TraceData.PrevStartLocation, 
		CurrentStart, 
		TraceData.TraceRadius, 
		TraceChannel, 
		false,
		ActorsToIgnore,
		DebugType, 
		StartHits, 
		true
	);
	CombinedHits.Append(StartHits);
	
	TArray<FHitResult> EndHits;
	UKismetSystemLibrary::SphereTraceMulti(
		this,
		TraceData.PrevEndLocation,
		CurrentEnd, TraceData.TraceRadius,
		TraceChannel,
		false,
		ActorsToIgnore,
		DebugType,
		EndHits,
		true
	);
	CombinedHits.Append(EndHits);
	
	TArray<FHitResult> CurrentHits;
	UKismetSystemLibrary::SphereTraceMulti(
		this,
		CurrentStart, 
		CurrentEnd, 
		TraceData.TraceRadius, 
		TraceChannel, 
		false, 
		ActorsToIgnore, 
		DebugType, 
		CurrentHits, 
		true
	);
	CombinedHits.Append(CurrentHits);
	
	CombinedHits.Sort([](const FHitResult& A, const FHitResult& B) {
		return A.Distance < B.Distance;
	});
	
	for (const FHitResult& HitResult : CombinedHits)
	{
		if (TraceData.MaxHitCount > 0 && TraceData.HitActors.Num() >= TraceData.MaxHitCount) break;
		
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor) continue;
		
		if (UPrimitiveComponent* HitComponent = HitResult.GetComponent())
		{
			if (HitComponent->GetCollisionObjectType() == ECC_WorldStatic) break;
		}
		
		if (!TraceData.HitActors.Contains(HitActor))
		{
			OnTargetHit(HitResult);
		}
	}
	
	TraceData.PrevStartLocation = CurrentStart;
	TraceData.PrevEndLocation = CurrentEnd;
}

void UKOGA_AttackBase::ResetHitActors()
{
	TraceData.HitActors.Empty();
}

UMeshComponent* UKOGA_AttackBase::FindTraceMesh()
{
	AKOCharacterBase* Character = Cast<AKOCharacterBase>(GetAvatarCharacter());
	if (!Character) return nullptr;
	
	UKOEquipmentComponent* EquipComp = Character->GetEquipmentComponent(); 
	
	if (EquipComp && EquipComp->HasWeapon())
	{
		AKOWeaponBase* WeaponActor = EquipComp->CurrentWeaponActor;
		if (UStaticMeshComponent* Mesh = WeaponActor->GetMesh())
		{
			if (Mesh->DoesSocketExist(TraceData.GetStartSocket())
				&& Mesh->DoesSocketExist(TraceData.GetEndSocket())) 
				return Mesh; 
		}
	}
	
	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	Character->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
		
	for (USkeletalMeshComponent* Mesh : SkeletalMeshes)
	{
		if (Mesh->DoesSocketExist(TraceData.GetStartSocket())
			&& Mesh->DoesSocketExist(TraceData.GetEndSocket()))
		{
			return Mesh; 
		}
	}
	
	return nullptr; 
}

void UKOGA_AttackBase::OnTargetHit(const FHitResult& Hit)
{
	KO_LOG(Combat, Warning, TEXT("OnTargetHit"));
	
	AActor* HitActor = Hit.GetActor();
	if (!Hit.bBlockingHit || !HitActor) return;
	
	KO_LOG(Combat, Warning, TEXT("HitActor : %s"), *HitActor->GetName());
	
	TraceData.HitActors.Add(HitActor);
	
	SendAttackEventsToTarget(HitActor);
	ApplyHitEffects(HitActor);
}

