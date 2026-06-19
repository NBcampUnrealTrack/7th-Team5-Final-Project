#include "UI/Map/KOMainMapWidget.h"

#include "Components/Image.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "KOMapUIComponent.h"

UKOMainMapWidget::UKOMainMapWidget()
{
	InputMode = EKOUIInputMode::All;
	bIsBackHandler = true;
}

void UKOMainMapWidget::SetMapUIComponent(UKOMapUIComponent* InMapUIComponent)
{
	MapUIComponent = InMapUIComponent;

	if (MapUIComponent)
	{
		LevelDimension = MapUIComponent->GetLevelDimension();
		CanvasDimension = MapUIComponent->GetMainMapCanvasDimension();
	}
}

void UKOMainMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdatePlayerIcon();
}

void UKOMainMapWidget::UpdatePlayerIcon()
{
	if (!ImagePlayerIcon || LevelDimension <= 0.f)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (!Pawn)
	{
		return;
	}

	const FVector Location = Pawn->GetActorLocation();
	const FRotator Rotation = Pawn->GetActorRotation();

	const float Scale = CanvasDimension / LevelDimension;

	ImagePlayerIcon->SetRenderTranslation(
		FVector2D(Location.X * Scale, Location.Y * Scale)
	);

	ImagePlayerIcon->SetRenderTransformAngle(Rotation.Yaw);
}