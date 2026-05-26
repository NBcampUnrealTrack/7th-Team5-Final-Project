#include "KOBaseBuilding.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Data/KODataTableTypes.h"
#include "GMRouterSubsystem.h"
#include "Items/KOItemLibrary.h"
#include "Messaging/KOMessageTypes.h"
#include "StructUtils/InstancedStruct.h"
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

	return UKOItemLibrary::GetFactoryRow(this, FactoryId);
}

bool AKOBaseBuilding::CanInteract_Implementation(AActor* /*Interactor*/) const
{
	return !FactoryId.IsNone();
}

void AKOBaseBuilding::OnInteract_Implementation(AActor* Interactor)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FKOBuildingInteractedMessage Msg;
	Msg.FactoryId  = FactoryId;
	Msg.Building   = this;
	Msg.Instigator = Interactor;

	UGMRouterSubsystem::BroadcastMessage(
		World,
		KOGameplayTags::Data_Message_Building_Interacted,
		FInstancedStruct::Make(Msg));

	UE_LOG(LogTemp, Log, TEXT("[Building] OnInteract 브로드캐스트: FactoryId=%s, Interactor=%s"),
		*FactoryId.ToString(),
		Interactor ? *Interactor->GetName() : TEXT("None"));
}

FText AKOBaseBuilding::GetInteractionPrompt_Implementation() const
{
	if (const FKOFactoryRow* Row = GetFactoryRow())
	{
		return Row->DisplayName;
	}
	return FText::GetEmpty();
}
