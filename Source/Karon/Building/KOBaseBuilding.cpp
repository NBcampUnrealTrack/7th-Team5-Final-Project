#include "KOBaseBuilding.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Component/Factory/KOEnergyProducerComponent.h"
#include "Component/Factory/KOFactoryProcessorComponent.h"
#include "Components/WidgetComponent.h"
#include "Subsystem/KOEnergySubsystem.h"
#include "Data/KODataTableTypes.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GMRouterSubsystem.h"
#include "Items/KOItemLibrary.h"

#include "StructUtils/InstancedStruct.h"
#include "UI/KOUISubsystem.h"
#include "Utility/Messaging/KOMessageTypes.h"

#include "Camera/PlayerCameraManager.h"
#include "Kismet/KismetMathLibrary.h"

AKOBaseBuilding::AKOBaseBuilding()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AKOBaseBuilding::BeginPlay()
{
	Super::BeginPlay();
	
	CachedProcessor = FindComponentByClass<UKOFactoryProcessorComponent>();

	PressureWarningWidget = FindComponentByClass<UWidgetComponent>();

	if (PressureWarningWidget)
	{
		PressureWarningWidget->SetHiddenInGame(true);
	}
}

void AKOBaseBuilding::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	RefreshPressureWarning();
	UpdatePressureWarningFacingCamera();
}

void AKOBaseBuilding::RefreshPressureWarning()
{
	if (!PressureWarningWidget)
	{
		return;
	}

	if (!bShowPressureWarning)
	{
		PressureWarningWidget->SetHiddenInGame(true);
		return;
	}

	if (!CachedProcessor)
	{
		PressureWarningWidget->SetHiddenInGame(true);
		return;
	}

	const bool bPressureAvailable = IsPressureAvailable();

	PressureWarningWidget->SetHiddenInGame(bPressureAvailable);
}

void AKOBaseBuilding::UpdatePressureWarningFacingCamera()
{
	if (!PressureWarningWidget)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->PlayerCameraManager)
	{
		return;
	}

	const FVector WidgetLocation = PressureWarningWidget->GetComponentLocation();
	const FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();

	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(WidgetLocation, CameraLocation);

	PressureWarningWidget->SetWorldRotation(LookAtRotation);
}

bool AKOBaseBuilding::IsPressureAvailable() const
{
	if (!CachedProcessor)
	{
		return true;
	}

	if (const UKOEnergySubsystem* Energy = UKOEnergySubsystem::Get(this))
	{
		return Energy->GetConsumerNetworkProductionRate(CachedProcessor) > KINDA_SMALL_NUMBER;
	}

	return false;
}

void AKOBaseBuilding::InitializeBuildingData(FName InFactoryId)
{
	if (InFactoryId.IsNone())
	{
		return;
	}

	FactoryId = InFactoryId;

	if (UKOEnergyProducerComponent* Producer = FindComponentByClass<UKOEnergyProducerComponent>())
	{
		Producer->InitializeFromRecipe();
	}
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
		// 위젯 열기. 닫기는 Back(팩토리 위젯의 bIsBackHandler).
		UKOUISubsystem::OpenWidget(this, WidgetTag);
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
