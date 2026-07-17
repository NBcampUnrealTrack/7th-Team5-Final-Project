#include "KOGA_AttackBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "MotionWarpingComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Ability/AbilityTask/AbilityTask_HitStop.h"
#include "AbilitySystem/Ability/AbilityTask/AbilityTask_Tick.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Effect/KOGameplayEffectContext.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/KOCharacterBase.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "Character/Enemy/Boss/KOBossBase.h"
#include "Component/Inventory/KOEquipmentComponent.h"
#include "Components/CapsuleComponent.h"
#include "Data/KO_HitData.h"
#include "GameFramework/Character.h"
#include "Items/Equipment/KOWeaponBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Utility/Log/KOLogManager.h"

UKOGA_AttackBase::UKOGA_AttackBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	AttackEventTags.AddTag(KOGameplayTags::Event_HitReact); 
	
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Character_Attacking);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Drawing);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_HitReacting);
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
	
	if (MotionWarpData.bUseMotionWarping)
	{
		UpdateMotionWarpTarget();
	}
	
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, 
			KOGameplayTags::Event_Trace_Start
		);
	
	WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnHitDataEventReceived);
	WaitEventTask->ReadyForActivation();
	

	TraceData.bIsFirstTick = true;
	TraceData.HitActors.Empty();
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
	
	if (MotionWarpData.bUseMotionWarping)
	{
		if (AKOCharacterBase* Character = Cast<AKOCharacterBase>(GetAvatarCharacter()))
		{
			if (UMotionWarpingComponent* MotionWarpingComponent = Character->GetMotionWarpingComponent())
			{
				MotionWarpingComponent->RemoveAllWarpTargets();
			}
		}
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
		EventData.OptionalObject = CachedHitData;
		
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
	Context.SetAbility(this);
	Context.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());
	
	if (FKOGameplayEffectContext* KOContext = static_cast<FKOGameplayEffectContext*>(Context.Get()))
	{
		KOContext->SetHitData(CachedHitData.Get());
	}
	
	for (const FKODamageEffectData& Effect : DamageEffects)
	{
		FGameplayEffectSpecHandle SpecHandle = 
		   SourceASC->MakeOutgoingSpec(Effect.EffectClass, Effect.Level, Context);
		if (!SpecHandle.IsValid()) continue;
		
		float FinalAttackCoefficient =  Effect.AttackCoefficient * CurrentDamageMultiplier;
		
		if (MontageData.IsValidIndex(CurrentMontageIndex)) 
			FinalAttackCoefficient *= MontageData[CurrentMontageIndex].DamageRate;
		
		SpecHandle.Data->SetSetByCallerMagnitude(
			KOGameplayTags::Data_AttackCoefficient, FinalAttackCoefficient); 
		
		
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
	
	for (const FKOEffectData& Effect : AdditionalEffects)
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

void UKOGA_AttackBase::OnHitDataEventReceived(FGameplayEventData Payload)
{
	if (const UKO_HitData* ReceivedData = Cast<UKO_HitData>(Payload.OptionalObject))
	{
		CachedHitData = ReceivedData;
		HitStopDuration = ReceivedData->HitData.HitStopDuration;
		HitStopTimeDilation = ReceivedData->HitData.HitStopTimeDilation;
        
		KO_LOG(Combat, Warning, TEXT("[ Hit Data ] KnockBackAmount: %f"), ReceivedData->HitData.KnockBackAmount);
	}
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
	AActor* HitActor = Hit.GetActor();
	if (!Hit.bBlockingHit || !HitActor) return;
	
	KO_LOG(Combat, Warning, TEXT("HitActor : %s"), *HitActor->GetName());
	
	UAbilitySystemComponent* TargetASC = 
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
	
	if (!TargetASC || TargetASC->HasMatchingGameplayTag(KOGameplayTags::State_Character_Dead)) return;
	TraceData.HitActors.Add(HitActor);
	
	ApplyHitEffects(HitActor);
	SendAttackEventsToTarget(HitActor);
	
	if (UAbilitySystemComponent* SourceASC = GetASC())
	{
		FGameplayCueParameters AttackerCueParams;
		AttackerCueParams.Location = Hit.ImpactPoint;
		AttackerCueParams.Normal = Hit.ImpactNormal;
		
		SourceASC->ExecuteGameplayCue(HitImpactAttackerCueTag, AttackerCueParams);
	}
	
	if (bUseHitStop)
	{
		UAbilityTask_HitStop* HitStopTask = UAbilityTask_HitStop::HitStop(
			this,
			HitActor,
			HitStopDuration,
			HitStopTimeDilation,
			true
		);
		HitStopTask->ReadyForActivation();
	}
}

AActor* UKOGA_AttackBase::FindMotionWarpTarget() const
{
	ACharacter* Character = GetAvatarCharacter();
	if (!Character) return nullptr;

	FVector Start = Character->GetActorLocation();
	FVector ForwardDir = Character->GetActorForwardVector();
	FVector End = Start + ForwardDir * MotionWarpData.MaxWarpDistance;
	
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Character);
	// 캐릭터에 부착물들도 Ignore 추가 
	TArray<AActor*> AttachedActors;
	Character->GetAttachedActors(AttachedActors);
	ActorsToIgnore.Append(AttachedActors);
	
	TArray<FHitResult> HitResults;
	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		this, Start, End,
		MotionWarpData.TargetSearchRange,
		UEngineTypes::ConvertToTraceType(ECC_Pawn), 
		false, ActorsToIgnore, 
		EDrawDebugTrace::None, 
		HitResults, true
	);
	
	if (!bHit) return nullptr; 
	
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor) continue;
		
		// 죽어 있는 대상은 WarpTarget 대상 x 
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!TargetASC || TargetASC->HasMatchingGameplayTag(KOGameplayTags::State_Character_Dead)) continue;
		
		return HitActor; 
	}
	
	return nullptr;
}

void UKOGA_AttackBase::UpdateMotionWarpTarget()
{
	AKOCharacterBase* Character = Cast<AKOCharacterBase>(GetAvatarCharacter());
	if (!Character) return;
	
	UMotionWarpingComponent* MotionWarpComp = Character->GetMotionWarpingComponent();
	if (!MotionWarpComp) return;
	
	AActor* TargetActor = FindMotionWarpTarget();
	
	if (!TargetActor || FVector::Dist(Character->GetActorLocation(), TargetActor->GetActorLocation()) > MotionWarpData.MaxWarpDistance)
	{
		MotionWarpComp->RemoveWarpTarget(MotionWarpData.TargetName);
		return;
	}
	
	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector AttackerLocation = Character->GetActorLocation();
	
	FVector2D Distance2D = FVector2D(TargetLocation - AttackerLocation);
	float Distance = Distance2D.Size();
	if (Distance <= KINDA_SMALL_NUMBER) return;
	
	FVector Direction = FVector(Distance2D, 0.f) / Distance;
	
	float AttackCapsuleRadius = 
		Character->GetCapsuleComponent() ? Character->GetCapsuleComponent()->GetScaledCapsuleRadius() : 0.f;
	
	float TargetCapsuleRadius = 0.0f;
	if (UCapsuleComponent* TargetCapsule = TargetActor->FindComponentByClass<UCapsuleComponent>())
		TargetCapsuleRadius = TargetCapsule->GetScaledCapsuleRadius();
	
	const float WarpStopDistance = AttackCapsuleRadius + TargetCapsuleRadius + MotionWarpData.ReachMargin;
	
	FVector FinalWarpLocation = Distance <= WarpStopDistance ? 
		AttackerLocation : TargetLocation - Direction * WarpStopDistance;
	
	FinalWarpLocation.Z = AttackerLocation.Z;
	FRotator FinalWarpRotation = Direction.Rotation();
	
	MotionWarpComp->AddOrUpdateWarpTargetFromLocationAndRotation(MotionWarpData.TargetName, FinalWarpLocation, FinalWarpRotation);
}

