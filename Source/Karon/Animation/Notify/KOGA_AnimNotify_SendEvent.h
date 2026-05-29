#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "KOGA_AnimNotify_SendEvent.generated.h"


UCLASS()
class KARON_API UKOGA_AnimNotify_SendEvent : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GAS")
	FGameplayTag EventTag;
	
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;
};
