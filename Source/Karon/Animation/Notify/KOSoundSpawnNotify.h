// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "KOSoundSpawnNotify.generated.h"

/**
 * 
 */
UCLASS()
class KARON_API UKOSoundSpawnNotify : public UAnimNotify
{
	GENERATED_BODY()
	UKOSoundSpawnNotify();
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, 
	UAnimSequenceBase* Animation, 
	const FAnimNotifyEventReference& EventReference) override;
	
protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> SoundBase;
	
	UPROPERTY(EditAnywhere)
	float Volume=1.f;
	
	UPROPERTY(EditAnywhere)
	float PlayRate=1.f;
	
	UPROPERTY(EditAnywhere)
	float StartTime=0.f;
};
