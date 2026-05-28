#include "KOBaseBuilding.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Component/KOEnergyProducerComponent.h"
#include "Component/KOFactoryProcessorComponent.h"
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
		// Interactor(Pawn) → PC → UISubsystem 경로로 토글. 인벤토리(Tab)와 같은 패턴.
		APlayerController* PC = nullptr;
		if (APawn* Pawn = Cast<APawn>(Interactor))
		{
			PC = Cast<APlayerController>(Pawn->GetController());
		}
		else
		{
			PC = Cast<APlayerController>(Interactor);
		}

		if (UKOUISubsystem* UISub = UKOUISubsystem::Get(PC))
		{
			UISub->ToggleWidget(WidgetTag);
		}
		else
		{
			// 폴백: UISubsystem 직접 접근 실패 시 기존 메시지 경로 사용.
			FKOUIPushLayerRequest UIReq;
			UIReq.WidgetTag = WidgetTag;
			UGMRouterSubsystem::BroadcastMessage(
				World,
				KOGameplayTags::Data_Message_UI_PushLayerRequest,
				FInstancedStruct::Make(UIReq));
		}
	}
}

FText AKOBaseBuilding::GetInteractionPrompt_Implementation() const
{
	if (const FKOFactoryRow* Row = GetFactoryRow())
	{
		return Row->DisplayName;
	}
	return FText::GetEmpty();
}
