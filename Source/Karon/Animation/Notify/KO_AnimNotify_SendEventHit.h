// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "KO_AnimNotify_SendEventHit.generated.h"

/**
 * 
 */
UCLASS()
class KARON_API UKO_AnimNotify_SendEventHit : public UAnimNotify
{
	GENERATED_BODY()
	UKO_AnimNotify_SendEventHit();
public:
	
	virtual void BranchingPointNotify(FBranchingPointNotifyPayload& BranchingPointPayload) override;
};
