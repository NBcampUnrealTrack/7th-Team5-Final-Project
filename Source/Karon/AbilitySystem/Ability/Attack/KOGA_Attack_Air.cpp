#include "KOGA_Attack_Air.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Ability/AbilityTask/AbilityTask_Tick.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "AbilitySystem/Tag/Input/KOGameplayTags_Input.h"
#include "AbilitySystem/Tag/State/KOGameplayTags_State.h"
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


UKOGA_Attack_Air::UKOGA_Attack_Air()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Attack_Light));
	ActivationRequiredTags.AddTag(KOGameplayTags::State_Character_Movement_InAir);
}

bool UKOGA_Attack_Air::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, 
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
		return false; 
	
	ACharacter* Character = GetAvatarCharacter();
	if (!Character) return false; 
	
	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	if (!Capsule) return false;
	
	FVector Start = Character->GetActorLocation() - 
		FVector(0.0f, 0.0f, Capsule->GetScaledCapsuleHalfHeight());
	
	FVector End = Start + FVector(0.0f, 0.0f, -3000);
	
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(Character);
	
	FHitResult Hit; 
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start, End,
		ECC_WorldStatic,
		CollisionParams
	);
	
	if (!bHit) return true; 
	
#if WITH_EDITOR || !UE_BUILD_SHIPPING
	if (TraceData.bShowDebug)
		DrawDebugDirectionalArrow(GetWorld(), Start,End, 10.f, FColor::Red);
#endif 
	
	return Hit.Distance >= MinHeight; 
}

void UKOGA_Attack_Air::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (MontageData.IsEmpty() || MontageSectionNames.Num() < 2)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return; 
	}
	
	ACharacter* Charcter = GetAvatarCharacter();
	if (!Charcter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return; 
	}
	
	ASC = GetASC(); 
	MovementComponent = Charcter->GetCharacterMovement(); 
	if (!MovementComponent || !ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return; 
	}
	
	CheckFallTask = UAbilityTask_Tick::CreateTickTask(this);
	CheckFallTask->OnTick.AddDynamic(this, &ThisClass::OnTick);
	CheckFallTask->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* TraceStartTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Trace_Start);
	
	TraceStartTask->EventReceived.AddDynamic(this, &ThisClass::OnTraceStart);
	TraceStartTask->ReadyForActivation(); 

	UAbilityTask_WaitGameplayEvent* TraceEndTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Trace_End);
	
	TraceEndTask->EventReceived.AddDynamic(this, &ThisClass::OnTraceEnd);
	TraceEndTask->ReadyForActivation(); 
	
	UAbilityTask_WaitGameplayEvent* GroundImpactTask  =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Attack_Air_GroundImpact);
	
	GroundImpactTask ->EventReceived.AddDynamic(this, &ThisClass::OnGroundImpact);
	GroundImpactTask ->ReadyForActivation();
	
	CurrentSectionIndex = 0;
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None,
			MontageData[0].Montage,
			MontageData[0].PlayRate
		);
	
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageTaskCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageTaskCancelled);
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageTaskCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageTaskCompleted);
	MontageTask->ReadyForActivation();
}

void UKOGA_Attack_Air::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (CheckFallTask)
	{
		CheckFallTask->EndTask();
		CheckFallTask = nullptr;
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_Attack_Air::OnTraceStart(FGameplayEventData Payload)
{
	ResetHitActors();
	
	TickTask = UAbilityTask_Tick::CreateTickTask(this);
	TickTask->OnTick.AddDynamic(this, &ThisClass::PerformWeaponTrace);
	TickTask->ReadyForActivation();
}

void UKOGA_Attack_Air::OnTraceEnd(FGameplayEventData Payload)
{
	if (TickTask)
	{
		TickTask->EndTask();
		TickTask = nullptr;
	}
}

void UKOGA_Attack_Air::OnGroundImpact(FGameplayEventData Payload)
{
	ACharacter* Character = GetAvatarCharacter();
	if (!Character || !ASC) return;
	
	FVector ImpactPoint = TraceData.TraceMesh ? 
	  TraceData.TraceMesh->GetSocketLocation(TraceData.GetEndSocket()) : 
	  Character->GetActorLocation();
	FHitResult GroundHit;
	
	bool bHitGround = FindGroundImpactPoint(ImpactPoint, GroundHit);
	if (GroundImpactCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = ImpactPoint;
		CueParams.Normal = bHitGround ? GroundHit.ImpactNormal : FVector::UpVector;
		CueParams.EffectContext = ASC->MakeEffectContext();
		if (bHitGround)
		{
			CueParams.EffectContext.AddHitResult(GroundHit);
		}
		CueParams.RawMagnitude = AttackRadius; 

		ASC->ExecuteGameplayCue(GroundImpactCueTag, CueParams);
	}
	
	ApplyRadialDamage(ImpactPoint, GroundHit);
}

void UKOGA_Attack_Air::OnMontageTaskCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Attack_Air::OnMontageTaskCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_Attack_Air::OnTick(float DeltaTime)
{
	if (!MovementComponent || !ASC) return;
	
	const int32 MaxSectionIndex = MontageSectionNames.Num() - 1;
	if (CurrentSectionIndex == MaxSectionIndex) return;
	
	if (MovementComponent->IsMovingOnGround()) 
	{
		HandleLandedState(MaxSectionIndex);
		return;
	}
	
	if (CurrentSectionIndex == 0 && MovementComponent->Velocity.Z <= 0.f)
	{
		CurrentSectionIndex = (CurrentSectionIndex+1) % MontageSectionNames.Num(); 
		ASC->CurrentMontageJumpToSection(MontageSectionNames[CurrentSectionIndex]);
		
		if (ACharacter* Character = GetAvatarCharacter())
		{
#if WITH_EDITOR || !UE_BUILD_SHIPPING
			if (TraceData.bShowDebug)
			{
				FVector DownwardStartLoc = TraceData.TraceMesh ? 
				   TraceData.TraceMesh->GetSocketLocation(TraceData.GetEndSocket()) : 
				   Character->GetActorLocation();
				
				DrawDebugSphere(GetWorld(), DownwardStartLoc, 25.f, 12, FColor::Red, false, 2.0f, 0, 2.0f);
			
				DrawDebugDirectionalArrow(GetWorld(), DownwardStartLoc, DownwardStartLoc + (FVector::DownVector * 100.f), 30.f, FColor::Red, false, 2.0f, 0, 3.0f);
			}
#endif
			
			Character->LaunchCharacter(
				FVector(0.f, 0.f, -FMath::Abs(DownwardForce)),
				false, true
			);
		}	
	}
}

void UKOGA_Attack_Air::HandleLandedState(int32 MaxSectionIndex)
{
	CurrentSectionIndex = MaxSectionIndex; 
	ASC->CurrentMontageJumpToSection(MontageSectionNames[CurrentSectionIndex]);
	
	ResetHitActors(); 
    
	if (CheckFallTask)
	{
		CheckFallTask->EndTask();
		CheckFallTask = nullptr;
	}
}

bool UKOGA_Attack_Air::FindGroundImpactPoint(FVector& OutImpactPoint, FHitResult& OutGroundHit)
{
	ACharacter* Character = GetAvatarCharacter();
	if (!Character) return false;
	
	FVector Start = TraceData.TraceMesh->GetSocketLocation(TraceData.GetEndSocket());
	OutImpactPoint = Start;
	
	FVector End = Start + (FVector::DownVector * 200.f);
    
	FCollisionQueryParams QueryParams;
	TArray<AActor*> AttachedActors;
    Character->GetAttachedActors(AttachedActors);
    QueryParams.AddIgnoredActors(AttachedActors);
	
	if (GetWorld()->LineTraceSingleByChannel(OutGroundHit, Start, End, ECC_Visibility, QueryParams))
	{
		OutImpactPoint = OutGroundHit.ImpactPoint;
		return true;
	}

	return false;
}

void UKOGA_Attack_Air::ApplyRadialDamage(const FVector& ImpactPoint, const FHitResult& GroundHit)
{
	ACharacter* Character = GetAvatarCharacter();
	if (!Character) return;
	
	const float TargetHalfHeight = 50.f;
	
#if WITH_EDITOR || !UE_BUILD_SHIPPING
	if (TraceData.bShowDebug)
	{
		FVector CylinderTop = ImpactPoint + FVector(0.f, 0.f, TargetHalfHeight);
		FVector CylinderBottom = ImpactPoint - FVector(0.f, 0.f, TargetHalfHeight);
        
		DrawDebugCylinder(
			GetWorld(), CylinderTop, CylinderBottom, AttackRadius, 24,
			FColor::Orange, false, 2.5f, 0, 1.5f
		);
	}
#endif
	
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape CylinderShape = FCollisionShape::MakeCapsule(AttackRadius, TargetHalfHeight);
    
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);
	TArray<AActor*> AttachedActors;
	Character->GetAttachedActors(AttachedActors);
	QueryParams.AddIgnoredActors(AttachedActors);

	bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		ImpactPoint,
		FQuat::Identity,
		ECC_Pawn,
		CylinderShape,
		QueryParams
	);

	if (!bHasOverlap) return;
	TArray<AActor*> ProcessedActors;
	
	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* HitActor = Result.GetActor();
		if (!HitActor || ProcessedActors.Contains(HitActor)) continue;
		
		ProcessedActors.Add(HitActor);
		
#if WITH_EDITOR || !UE_BUILD_SHIPPING
		if (TraceData.bShowDebug)
		{
			DrawDebugPoint(GetWorld(), HitActor->GetActorLocation(), 20.f, FColor::Green, false, 2.5f);
		}
#endif

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!TargetASC || TargetASC->HasMatchingGameplayTag(KOGameplayTags::State_Character_Dead)) continue;
	
		SendAttackEventsToTarget(HitActor);
		ApplyHitEffects(HitActor);
	}
}


