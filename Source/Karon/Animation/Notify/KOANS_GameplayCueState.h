#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "KOANS_GameplayCueState.generated.h"


UCLASS()
class KARON_API UKOANS_GameplayCueState : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation, float TotalDuration,
		const FAnimNotifyEventReference& EventReference
	) override;
	
	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp, 
		UAnimSequenceBase* Animation, 
		const FAnimNotifyEventReference& EventReference
	) override;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayCue, meta = (Categories = "GameplayCue"))
	FGameplayTag CueTag;

};
