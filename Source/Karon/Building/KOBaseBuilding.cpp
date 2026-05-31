#include "KOBaseBuilding.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Component/Factory/KOEnergyProducerComponent.h"
#include "Component/Factory/KOFactoryProcessorComponent.h"
#include "Data/KODataTableTypes.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GMRouterSubsystem.h"
#include "Items/KOItemLibrary.h"
#include "Messaging/KOMessageTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "UI/KOUISubsystem.h"

AKOBaseBuilding::AKOBaseBuilding()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AKOBaseBuilding::InitializeBuildingData(FName InFactoryId)
{
	if (InFactoryId.IsNone())
	{
		return;
	}

	FactoryId = InFactoryId;
}

const FKOFactoryRow* AKOBaseBuilding::GetFactoryRow() const
{
	if (FactoryId.IsNone())
	{
		return nullptr;
	}

	return UKOItemLibrary::GetFactoryRow(this, FactoryId);
}

bool AKOBaseBuilding::CanInteract(AActor* /*Interactor*/) const
{
	return !FactoryId.IsNone();
}

void AKOBaseBuilding::OnInteract(AActor* Interactor)
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

	// 컴포넌트 유무로 표시할 UI 결정 (Processor / Producer는 상호 배타).
	FGameplayTag WidgetTag;
	if (FindComponentByClass<UKOFactoryProcessorComponent>())
	{
		WidgetTag = KOGameplayTags::UI_Widget_Factory_Processor;
	}
	else if (FindComponentByClass<UKOEnergyProducerComponent>())
	{
		WidgetTag = KOGameplayTags::UI_Widget_Factory_Producer;
	}

	if (WidgetTag.IsValid())
	{
		// 호출 일원화: GMS 경로로 위젯 열기 요청. 닫기는 Back(팩토리 위젯의 bIsBackHandler).
		UKOUISubsystem::RequestOpenWidget(this, WidgetTag);
	}
}

FText AKOBaseBuilding::GetInteractionPrompt() const
{
	if (const FKOFactoryRow* Row = GetFactoryRow())
	{
		return Row->DisplayName;
	}
	return FText::GetEmpty();
}
