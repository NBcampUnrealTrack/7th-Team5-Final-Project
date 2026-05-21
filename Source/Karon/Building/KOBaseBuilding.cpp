#include "KOBaseBuilding.h"
#include "Data/KOBuildingDataAsset.h"

AKOBaseBuilding::AKOBaseBuilding()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AKOBaseBuilding::InitializeBuildingData(UKOBuildingDataAsset* InBuildingData)
{
	if (!InBuildingData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Building] InitializeBuildingData 실패: BuildingData가 없습니다."));
		return;
	}

	BuildingData = InBuildingData;

	UE_LOG(LogTemp, Log, TEXT("[Building] BuildingData 초기화 완료: %s"), *BuildingData->GetName());
}