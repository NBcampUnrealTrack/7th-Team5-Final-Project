#pragma once

#include "CoreMinimal.h"
#include "NiagaraAnimNotifies/Public/AnimNotifyState_TimedNiagaraEffect.h"
#include "KO_AnimNotifyState_NiagaraEffectAdvanced.generated.h"

UCLASS()
class KARON_API UKO_AnimNotifyState_NiagaraEffectAdvanced : public UAnimNotifyState_TimedNiagaraEffectAdvanced
{
	GENERATED_BODY()
	
public:
	virtual void NotifyTick(
		USkeletalMeshComponent* MeshComp, 
		UAnimSequenceBase* Animation, 
		float FrameDeltaTime, 
		const FAnimNotifyEventReference& EventReference
	) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trail")
	FName StartSocketName = FName("TraceStart");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trail")
	FName EndSocketName = FName("TraceEnd");
};
