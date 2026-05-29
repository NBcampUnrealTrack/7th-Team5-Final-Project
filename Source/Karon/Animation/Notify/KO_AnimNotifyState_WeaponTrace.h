// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "KO_AnimNotifyState_WeaponTrace.generated.h"

/**
 * 
 */
UCLASS()
class KARON_API UKO_AnimNotifyState_WeaponTrace : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference
	) override;
	
	virtual void NotifyTick(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference
	) override;
	
	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;
	
protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trace")
	FName StartSocketName = "TraceStart";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trace")
	FName EndSocketName = "TraceEnd";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trace")
	float TraceRadius = 20.0f;
	
	UPROPERTY()
	TArray<AActor*> HitActors;
	
};
