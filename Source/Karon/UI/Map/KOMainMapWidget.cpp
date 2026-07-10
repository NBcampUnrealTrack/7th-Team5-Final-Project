#include "UI/Map/KOMainMapWidget.h"
#include "KOMapUIComponent.h"

#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/PanelWidget.h"
#include "Components/PanelSlot.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/OverlaySlot.h"
#include "Components/BorderSlot.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBoxSlot.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"

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
		LevelDimensionX = MapUIComponent->GetFogMapSize().X;
		LevelDimensionY = MapUIComponent->GetFogMapSize().Y;
		CanvasDimension = MapUIComponent->GetMainMapCanvasDimension();
		MapOrigin = MapUIComponent->GetFogMapOrigin();
	}

	ApplyFogParams();

	CurrentZoom = 1.f;
	PanOffset = FVector2D::ZeroVector;
	if (MapContentSizeBox)
	{
		// WidthOverride/HeightOverride는 고정 레이아웃 크기이며 줌에 따라 다시 바뀌지 않는다.
		// 확대/축소는 RenderScale로만 처리한다 (아래 NativeOnMouseWheel 참고).
		MapContentSizeBox->SetWidthOverride(MapPixelSize.X);
		MapContentSizeBox->SetHeightOverride(MapPixelSize.Y);
		MapContentSizeBox->SetRenderScale(FVector2D(CurrentZoom, CurrentZoom));
		ApplyPanOffset();
	}

	bPendingCenterOnOpen = true;
}

void UKOMainMapWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (MapContentSizeBox == nullptr)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("KOMainMapWidget: MapContentSizeBox 바인딩 실패 ")
		       TEXT("(WBP_MainMap의 SizeBox 이름이 'MapContentSizeBox'와 정확히 일치하는지 확인하세요). ")
		       TEXT("드래그 팬/휠 줌이 동작하지 않습니다."));
	}
	else if (MapContentSizeBox->GetParent() == nullptr)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("KOMainMapWidget: MapContentSizeBox의 부모 패널을 찾을 수 없습니다. ")
		       TEXT("뷰포트 크기를 알 수 없어 중앙 정렬/클램프가 동작하지 않습니다."));
	}

	EnsureTopLeftAlignment();

	if (FogOverlayMaterial == nullptr || ImageFogOverlay == nullptr)
	{
		return;
	}

	FogOverlayMID = UMaterialInstanceDynamic::Create(FogOverlayMaterial, this);
	ImageFogOverlay->SetBrushFromMaterial(FogOverlayMID);

	bFogParamsBound = false;
}

void UKOMainMapWidget::ApplyFogParams()
{
	if (FogOverlayMID == nullptr || MapUIComponent == nullptr || bFogParamsBound)
	{
		return;
	}

	UTextureRenderTarget2D* ExploredRT = MapUIComponent->GetExploredFogRT();
	UTextureRenderTarget2D* VisibleRT = MapUIComponent->GetVisibleFogRT();

	if (ExploredRT == nullptr || VisibleRT == nullptr)
	{
		return;
	}

	FogOverlayMID->SetTextureParameterValue(TEXT("ExploredMask"), ExploredRT);
	FogOverlayMID->SetTextureParameterValue(TEXT("VisibleMask"), VisibleRT);

	bFogParamsBound = true;
}

void UKOMainMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bFogParamsBound == false)
	{
		ApplyFogParams();
	}

	UpdatePlayerIcon();

	if (bPendingCenterOnOpen)
	{
		CenterViewOnPlayer();
	}
}

FReply UKOMainMapWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && MapContentSizeBox)
	{
		bIsDraggingMap = true;
		return FReply::Handled().CaptureMouse(TakeWidget()).SetUserFocus(TakeWidget());
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UKOMainMapWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDraggingMap && MapContentSizeBox)
	{
		// RenderTranslation 기반이므로 ScrollOffset과 달리 커서 이동 방향 그대로 더한다
		// (콘텐츠가 마우스를 따라 움직인다).
		PanOffset += InMouseEvent.GetCursorDelta();
		ApplyPanOffset();

		return FReply::Handled();
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply UKOMainMapWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsDraggingMap)
	{
		bIsDraggingMap = false;
		return FReply::Handled().ReleaseMouseCapture();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UKOMainMapWidget::NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent)
{
	bIsDraggingMap = false;
	Super::NativeOnMouseCaptureLost(CaptureLostEvent);
}

FReply UKOMainMapWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (MapContentSizeBox == nullptr)
	{
		return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
	}

	const float OldZoom = CurrentZoom;
	const float NewZoom = FMath::Clamp(CurrentZoom + InMouseEvent.GetWheelDelta() * ZoomStep, MinZoom, MaxZoom);
	if (FMath::IsNearlyEqual(NewZoom, OldZoom))
	{
		return FReply::Handled();
	}

	// WidthOverride/HeightOverride는 더 이상 바꾸지 않으므로(고정 레이아웃 크기) 커서 아래의
	// 콘텐츠 로컬 좌표(P)는 줌과 무관하게 항상 같은 좌표계(0..BaseContentSize)를 가리킨다.
	// 그 지점이 커서 아래에 그대로 유지되도록(=커서 기준 줌) PanOffset을 보정한다:
	//   PanOffset_new = PanOffset_old - P * (NewZoom - OldZoom)
	const FVector2D CursorScreenPos = InMouseEvent.GetScreenSpacePosition();
	const FVector2D ContentLocalPos = MapContentSizeBox->GetCachedGeometry().AbsoluteToLocal(CursorScreenPos);

	PanOffset -= ContentLocalPos * (NewZoom - OldZoom);

	CurrentZoom = NewZoom;
	MapContentSizeBox->SetRenderScale(FVector2D(CurrentZoom, CurrentZoom));
	ApplyPanOffset();

	return FReply::Handled();
}

void UKOMainMapWidget::UpdatePlayerIcon()
{
	if (ImagePlayerIcon == nullptr || LevelDimensionX <= 0.f || LevelDimensionY <= 0.f)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (Pawn == nullptr)
	{
		return;
	}

	const FVector Location = Pawn->GetActorLocation();
	const FRotator Rotation = Pawn->GetActorRotation();

	const FVector2D CanvasPos = ComputePlayerCanvasPos(Location);

	// RenderTranslation 대신 실제 슬롯의 Position을 제어하는 것이 스크롤 박스 안에서 안전합니다.
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ImagePlayerIcon->Slot))
	{
		CanvasSlot->SetPosition(CanvasPos);
	}

	// 아이콘 아트가 Yaw=0 기준과 다른 방향을 바라보고 있을 때를 위한 디자이너 튜닝용 각도 보정.
	ImagePlayerIcon->SetRenderTransformAngle(Rotation.Yaw + CorrectionRotation);
}

FVector2D UKOMainMapWidget::ComputePlayerCanvasPos(const FVector& WorldLocation) const
{
	if (LevelDimensionX <= 0.f || LevelDimensionY <= 0.f) return FVector2D::ZeroVector;

	// MapPixelSize가 곧 MapContentSizeBox(=지도 텍스처)의 실제 픽셀 크기이므로, 월드 좌표를
	// 그 크기 기준으로 직접 변환하면 된다. 여기에 별도 배율을 추가로 곱하면(예: 예전에 쓰던
	// BaseContentSize/CanvasDimension 배율) 이미 반영된 크기를 이중으로 스케일링하게 되어
	// 위치가 미세하게 어긋난다.
	const float ScaleX = MapPixelSize.X / LevelDimensionX;
	const float ScaleY = MapPixelSize.Y / LevelDimensionY;

	return FVector2D(
		(WorldLocation.X - MapOrigin.X) * ScaleX,
		(WorldLocation.Y - MapOrigin.Y) * ScaleY
	);
}

FVector2D UKOMainMapWidget::GetViewportSize() const
{
	if (MapContentSizeBox == nullptr)
	{
		return FVector2D::ZeroVector;
	}

	UPanelWidget* ParentPanel = MapContentSizeBox->GetParent();
	return ParentPanel ? ParentPanel->GetCachedGeometry().GetLocalSize() : FVector2D::ZeroVector;
}

FVector2D UKOMainMapWidget::ClampPanOffset(const FVector2D& InPanOffset) const
{
	const FVector2D ViewportSize = GetViewportSize();
	const FVector2D ContentSize = MapPixelSize * CurrentZoom;

	if (ViewportSize.X <= 0.f || ViewportSize.Y <= 0.f || ContentSize == FVector2D::ZeroVector)
	{
		return InPanOffset;
	}

	// 콘텐츠가 뷰포트보다 크면 [뷰포트-콘텐츠, 0] 범위로, 작으면 [0, 뷰포트-콘텐츠] 범위로 팬을 제한해
	// 어느 경우든 콘텐츠가 뷰포트 밖으로 완전히 사라지지 않게 한다.
	const float MinX = FMath::Min(0.f, ViewportSize.X - ContentSize.X);
	const float MaxX = FMath::Max(0.f, ViewportSize.X - ContentSize.X);
	const float MinY = FMath::Min(0.f, ViewportSize.Y - ContentSize.Y);
	const float MaxY = FMath::Max(0.f, ViewportSize.Y - ContentSize.Y);

	return FVector2D(
		FMath::Clamp(InPanOffset.X, MinX, MaxX),
		FMath::Clamp(InPanOffset.Y, MinY, MaxY)
	);
}

void UKOMainMapWidget::ApplyPanOffset()
{
	if (MapContentSizeBox == nullptr)
	{
		return;
	}

	PanOffset = ClampPanOffset(PanOffset);
	MapContentSizeBox->SetRenderTranslation(PanOffset);
}

void UKOMainMapWidget::CenterViewOnPlayer()
{
	if (MapContentSizeBox == nullptr)
	{
		// 이건 재시도해도 소용없는 영구적인 실패 조건이라 여기서만 포기한다.
		bPendingCenterOnOpen = false;
		return;
	}

	// LevelDimension/뷰포트 크기는 아직 준비되지 않았을 수 있는 "일시적" 조건이므로,
	// 여기서 bPendingCenterOnOpen을 끄지 않고 그냥 return해서 다음 틱에 다시 시도한다.
	if (LevelDimensionX <= 0.f || LevelDimensionY <= 0.f)
	{
		return;
	}

	// 뷰포트(=MapContentSizeBox 부모 패널) 크기는 레이아웃이 한 번 계산된 뒤에만 유효하므로,
	// 아직 크기가 0이면(=위젯이 막 열려 지오메트리가 확정되지 않음) 다음 틱에 다시 시도한다.
	const FVector2D ViewportSize = GetViewportSize();
	if (ViewportSize.X <= 0.f || ViewportSize.Y <= 0.f)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (Pawn == nullptr)
	{
		return;
	}

	const FVector Location = Pawn->GetActorLocation();
	const FVector2D CanvasPos = ComputePlayerCanvasPos(Location);

	// CanvasPos는 줌과 무관한 고정 좌표계이므로, 화면상 위치를 구하려면 CurrentZoom을 곱해야 한다
	// (ScreenPos = PanOffset + CanvasPos * CurrentZoom, MapContentSizeBox 좌상단은 뷰포트 (0,0)에 고정).
	// 플레이어가 뷰포트 정중앙에 오도록 PanOffset을 역산한다.
	PanOffset = ViewportSize * 0.5f - CanvasPos * CurrentZoom;
	ApplyPanOffset();

	UE_LOG(LogTemp, Log,
	       TEXT("KOMainMapWidget: CenterViewOnPlayer 적용 - ViewportSize=(%.1f,%.1f) CanvasPos=(%.1f,%.1f) ")
	       TEXT(
		       "CurrentZoom=%.2f -> PanOffset=(%.1f,%.1f) (클램프 후) / LevelDim=(%.1f,%.1f) MapOrigin=(%.1f,%.1f) CanvasDimension=%.1f"
	       ),
	       ViewportSize.X, ViewportSize.Y, CanvasPos.X, CanvasPos.Y, CurrentZoom, PanOffset.X, PanOffset.Y,
	       LevelDimensionX, LevelDimensionY, MapOrigin.X, MapOrigin.Y, CanvasDimension);
	UE_LOG(LogTemp, Warning,
	TEXT("CanvasPos=(%.1f %.1f)"),
	CanvasPos.X,
	CanvasPos.Y);
	UE_LOG(LogTemp, Warning,
	TEXT("PlayerWorld=(%.0f %.0f)"),
	Location.X,
	Location.Y);
	bPendingCenterOnOpen = false;
}

void UKOMainMapWidget::EnsureTopLeftAlignment()
{
	if (!MapContentSizeBox)
	{
		return;
	}

	// 스케일이 좌상단 기준으로 커지도록 피벗을 고정한다 (기본값 0.5,0.5는 중앙 기준 확대라
	// PanOffset 계산과 어긋난다).
	MapContentSizeBox->SetRenderTransformPivot(FVector2D::ZeroVector);

	UPanelSlot* ContentSlot = MapContentSizeBox->Slot;
	if (!ContentSlot)
	{
		return;
	}

	if (UCanvasPanelSlot* AsCanvas = Cast<UCanvasPanelSlot>(ContentSlot))
	{
		// 점 앵커(좌상단)로 고정하고 위치 오프셋도 0으로 만들어야, PanOffset(RenderTranslation) 계산이
		// 가정하는 "부모 뷰포트의 (0,0)에 기준점이 고정되어 있다"는 전제가 성립한다.
		AsCanvas->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
		AsCanvas->SetAlignment(FVector2D::ZeroVector);
		AsCanvas->SetPosition(FVector2D::ZeroVector);
	}
	else if (UOverlaySlot* AsOverlay = Cast<UOverlaySlot>(ContentSlot))
	{
		AsOverlay->SetHorizontalAlignment(HAlign_Left);
		AsOverlay->SetVerticalAlignment(VAlign_Top);
	}
	else if (UBorderSlot* AsBorder = Cast<UBorderSlot>(ContentSlot))
	{
		AsBorder->SetHorizontalAlignment(HAlign_Left);
		AsBorder->SetVerticalAlignment(VAlign_Top);
	}
	else if (UVerticalBoxSlot* AsVBox = Cast<UVerticalBoxSlot>(ContentSlot))
	{
		AsVBox->SetHorizontalAlignment(HAlign_Left);
		AsVBox->SetVerticalAlignment(VAlign_Top);
	}
	else if (UHorizontalBoxSlot* AsHBox = Cast<UHorizontalBoxSlot>(ContentSlot))
	{
		AsHBox->SetHorizontalAlignment(HAlign_Left);
		AsHBox->SetVerticalAlignment(VAlign_Top);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("KOMainMapWidget: MapContentSizeBox의 부모 슬롯 타입(%s)에서는 정렬을 코드로 강제하지 못합니다. ")
		       TEXT("에디터에서 이 슬롯의 Horizontal/Vertical Alignment를 직접 Left/Top으로 맞춰주세요."),
		       *ContentSlot->GetClass()->GetName());
	}
}
