#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "KOEnemyAttackNotifyState.generated.h"

class UKOEnemyAttackGameplayAbility;

UCLASS()
class KARON_API UKOEnemyAttackNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	UKOEnemyAttackNotifyState();
	
public:
	virtual void BranchingPointNotifyBegin(FBranchingPointNotifyPayload& BranchingPointPayload) override;
	virtual void BranchingPointNotifyEnd(FBranchingPointNotifyPayload& BranchingPointPayload) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	
protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Enemy|Attack")
	float TraceRadius=50.0f;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Enemy|Attack")
	bool bShowDebug=true;

private:
	//소켓이름
	FName SocketName=FName("Socket");
	UPROPERTY()
	TMap<USkeletalMeshComponent*,UKOEnemyAttackGameplayAbility*> CachedAbilities;
};
