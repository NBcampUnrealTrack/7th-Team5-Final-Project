// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "KO_AnimNotify_CameraShake.generated.h"

/**
 * 
 */
UCLASS()
class KARON_API UKO_AnimNotify_CameraShake : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation, 
		const FAnimNotifyEventReference& EventReference
	) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = CameraShake)
	TSubclassOf<UCameraShakeBase> ShakeClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = CameraShake)
	float Scale = 1.f;
};
