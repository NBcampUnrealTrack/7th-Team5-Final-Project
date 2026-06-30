#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "KO_AnimNotifyState_SendEvent.generated.h"


UCLASS()
class KARON_API UKO_AnimNotifyState_SendEvent : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	UKO_AnimNotifyState_SendEvent();
	
	virtual void BranchingPointNotifyBegin(FBranchingPointNotifyPayload& BranchingPointPayload) override;
	virtual void BranchingPointNotifyEnd(FBranchingPointNotifyPayload& BranchingPointPayload) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag StartTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag EndTag;
};
