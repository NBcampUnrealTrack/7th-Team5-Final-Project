// Fill out your copyright notice in the Description page of Project Settings.


#include "KO_AnimNotifyState_WeaponTrace.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"

void UKO_AnimNotifyState_WeaponTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                  float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	HitActors.Empty();
}

void UKO_AnimNotifyState_WeaponTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	
	if (!MeshComp || !MeshComp->GetWorld())
	{
		return;
	}
	
	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}
	
	FVector StartLocation = MeshComp->GetSocketLocation(StartSocketName);
	FVector EndLocation = MeshComp->GetSocketLocation(EndSocketName);
	
	TArray<FHitResult> HitResults;
	
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(TraceRadius);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);
	QueryParams.AddIgnoredActors(HitActors);
	
	bool bHit = MeshComp->GetWorld()->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECC_Pawn,
		CollisionShape,
		QueryParams
	);
	
	DrawDebugCapsule(
		MeshComp->GetWorld(),
		(StartLocation + EndLocation) * 0.5f,
		FVector::Dist(StartLocation, EndLocation) * 0.5f,
		TraceRadius,
		FRotationMatrix::MakeFromZ(EndLocation - StartLocation).ToQuat(),
		bHit ? FColor::Red : FColor::Green,
		false,
		1.0f
	);
	
	if (bHit)
	{
		for (const FHitResult& HitResult : HitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			
			if (HitActor && !HitActors.Contains(HitActor))
			{
				HitActors.Add(HitActor);
				
				FGameplayEventData PayloadData;
				PayloadData.Instigator = OwnerActor;
				PayloadData.Target = HitActor;
				
				FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Hit"));
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EventTag, PayloadData);
			}
		}
	}
}

void UKO_AnimNotifyState_WeaponTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	HitActors.Empty();
}
