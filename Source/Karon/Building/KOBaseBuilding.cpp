#include "KOBaseBuilding.h"
#include "Data/KODataTableTypes.h"
#include "Subsystem/KOLoadSubsystem.h"

AKOBaseBuilding::AKOBaseBuilding()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AKOBaseBuilding::InitializeBuildingData(FName InFactoryId)
{
	if (InFactoryId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Building] InitializeBuildingData 실패: FactoryId가 비어 있습니다."));
		return;
	}

	FactoryId = InFactoryId;

	UE_LOG(LogTemp, Log, TEXT("[Building] FactoryId 초기화 완료: %s"), *FactoryId.ToString());
}

const FKOFactoryRow* AKOBaseBuilding::GetFactoryRow() const
{
	if (FactoryId.IsNone())
	{
		return nullptr;
	}

	const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
	return LoadSub ? LoadSub->FindFactoryRow(FactoryId) : nullptr;
}
