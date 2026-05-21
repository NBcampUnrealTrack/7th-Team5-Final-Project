#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOBaseBuilding.generated.h"

class UKOBuildingDataAsset;

UCLASS()
class KARON_API AKOBaseBuilding : public AActor
{
	GENERATED_BODY()

public:
	AKOBaseBuilding();

public:
	// 건설 직후 BuildComponent가 DataAsset을 넘겨주는 함수
	UFUNCTION(BlueprintCallable, Category = "Building")
	void InitializeBuildingData(UKOBuildingDataAsset* InBuildingData);

	UFUNCTION(BlueprintPure, Category = "Building")
	UKOBuildingDataAsset* GetBuildingData() const { return BuildingData; }

protected:
	// 실제 월드에 설치된 건물이 참조할 데이터
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Building")
	TObjectPtr<UKOBuildingDataAsset> BuildingData;
};