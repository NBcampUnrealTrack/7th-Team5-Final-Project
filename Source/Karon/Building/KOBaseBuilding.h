#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utility/Interface/KOInteractableInterface.h"

#include "KOBaseBuilding.generated.h"

struct FKOFactoryRow;
class UWidgetComponent;
class UKOFactoryProcessorComponent;

UCLASS()
class KARON_API AKOBaseBuilding : public AActor, public IKOInteractableInterface
{
	GENERATED_BODY()

public:
	AKOBaseBuilding();

public:
	// 건설 직후 BuildComponent가 FactoryId를 넘겨주는 함수
	void InitializeBuildingData(FName InFactoryId);

	FName GetFactoryId() const { return FactoryId; }

	/** LoadSubsystem을 통해 자신이 참조하는 Factory Row를 조회 */
	const FKOFactoryRow* GetFactoryRow() const;

	// ─── IKOInteractableInterface ─────────────────────────────────────────────
	virtual bool  CanInteract(AActor* Interactor) const override;
	virtual void  OnInteract(AActor* Interactor) override;
	virtual FText GetInteractionPrompt() const override;
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KO|Energy")
	bool bShowPressureWarning = true;

	// 실제 월드에 설치된 건물이 참조할 Factory DataTable의 RowName
	UPROPERTY(VisibleInstanceOnly, Category = "Building")
	FName FactoryId = NAME_None;

private:
	UPROPERTY()
	TObjectPtr<UWidgetComponent> PressureWarningWidget = nullptr;

	UPROPERTY()
	TObjectPtr<UKOFactoryProcessorComponent> CachedProcessor = nullptr;

	void RefreshPressureWarning(); // 표시, 숨김
	bool IsPressureAvailable() const; // 압력 체크
	
	void UpdatePressureWarningFacingCamera();
};
