#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "KO_AnimNotify_SendGameplayEvent.generated.h"


UCLASS()
class KARON_API UKO_AnimNotify_SendGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UKO_AnimNotify_SendGameplayEvent();
	
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;
	
	virtual FString GetNotifyName_Implementation() const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Event")
	FGameplayTag EventTag;
};
