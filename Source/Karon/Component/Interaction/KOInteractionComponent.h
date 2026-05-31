// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KOInteractionComponent.generated.h"

/**
 * UKOInteractionComponent
 * PlayerController에 부착되어 화면 중앙 라인트레이스로 IKOInteractableInterface를 구현한
 * 액터를 매 프레임 갱신하고, 입력 액션이 들어오면 OnInteract를 호출한다.
 */
UCLASS(ClassGroup=(KO), meta=(BlueprintSpawnableComponent))
class KARON_API UKOInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKOInteractionComponent();

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	/** 입력 키 호출 시 PlayerController에서 호출 */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	bool TryInteract();

	/** 현재 조준 중인 상호작용 대상 (없으면 nullptr) */
	AActor* GetCurrentInteractable() const { return CurrentInteractable.Get(); }

protected:
	bool TraceFromScreenCenter(FHitResult& OutHit) const;
	void UpdateCurrentInteractable();

protected:
	UPROPERTY(EditAnywhere, Category = "Interaction|Trace")
	float TraceDistance = 500.f;

	/** 스피어 트레이스 반경(cm). 0 이하이면 라인 트레이스로 폴백. */
	UPROPERTY(EditAnywhere, Category = "Interaction|Trace")
	float TraceRadius = 20.f;

	UPROPERTY(EditAnywhere, Category = "Interaction|Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

private:
	TWeakObjectPtr<AActor> CurrentInteractable;
};
