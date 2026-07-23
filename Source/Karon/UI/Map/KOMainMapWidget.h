#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOMainMapWidget.generated.h"

class UImage;
class USizeBox;
class UPanelWidget;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UKOMapUIComponent;

UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOMainMapWidget : public UKOActivatableWidget
{
	GENERATED_BODY()

public:
	UKOMainMapWidget();

	void SetMapUIComponent(UKOMapUIComponent* InMapUIComponent);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImagePlayerIcon;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> ImageFogOverlay;

	/**
	 * 맵 콘텐츠(CanvasPanel)를 감싸는 SizeBox. 드래그 팬(RenderTranslation)과 휠 줌
	 * (Width/HeightOverride) 기능을 모두 이 위젯 하나로 처리한다. ScrollBox는 쓰지 않으며,
	 * 이 위젯의 부모 패널이 곧 "보여지는 화면(뷰포트)"이므로 그 부모 패널의 Clipping을
	 * Clip to Bounds로 설정해야 콘텐츠가 화면 밖으로 넘칠 때 잘려 보인다.
	 */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<USizeBox> MapContentSizeBox;

	UPROPERTY(EditDefaultsOnly, Category="KO|Map")
	TObjectPtr<UMaterialInterface> FogOverlayMaterial;

	UPROPERTY(EditDefaultsOnly, Category="KO|Map|Zoom")
	float MinZoom = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category="KO|Map|Zoom")
	float MaxZoom = 2.5f;

	UPROPERTY(EditDefaultsOnly, Category="KO|Map|Zoom")
	float ZoomStep = 0.1f;

	/**
	 * CurrentZoom == 1.0 일 때 MapContentSizeBox의 Width/HeightOverride 값(Slate 단위) = 지도
	 * 텍스처의 실제 픽셀 크기. 월드 좌표 -> 캔버스 좌표 변환의 유일한 배율 기준이다.
	 */
	UPROPERTY(EditDefaultsOnly, Category="KO|Map|Zoom")
	FVector2D MapPixelSize = FVector2D(2599.f, 2600.f);

	/** 아이콘 아트가 Yaw=0(월드 +X 방향)과 다른 방향을 바라보고 있을 때 보정할 각도. */
	UPROPERTY(EditDefaultsOnly, Category = "KO|Map|PlayerIcon")
	float CorrectionRotation = 0.f;

private:
	void UpdatePlayerIcon();
	void ApplyFogParams();
	void CenterViewOnPlayer();
	FVector2D ComputePlayerCanvasPos(const FVector& WorldLocation) const;

	/** MapContentSizeBox의 부모 패널(=클리핑되는 뷰포트) 크기. 아직 레이아웃이 안 잡혔으면 (0,0). */
	FVector2D GetViewportSize() const;

	/** 콘텐츠가 뷰포트 밖으로 완전히 벗어나지 않도록 PanOffset을 보정한다. */
	FVector2D ClampPanOffset(const FVector2D& InPanOffset) const;

	void ApplyPanOffset();

	/**
	 * 팬/줌 계산은 "MapContentSizeBox의 좌상단이 부모 뷰포트의 좌상단(0,0)에 고정되어 있다"고
	 * 가정한다. 부모 슬롯의 정렬이 Center 등으로 되어 있으면 크기/스케일이 바뀔 때마다 앵커
	 * 위치 자체가 움직여서 팬/줌이 대각선으로 어긋나 보이므로, 알려진 슬롯 타입에 한해
	 * 좌상단 정렬을 코드에서 강제한다.
	 */
	void EnsureTopLeftAlignment();

	UPROPERTY()
	TObjectPtr<UKOMapUIComponent> MapUIComponent;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FogOverlayMID;

	float LevelDimensionX = 150310.f;
	float LevelDimensionY = 144460.f;
	float CanvasDimension = 1000.f;

	FVector2D MapOrigin = FVector2D::ZeroVector;

	bool bFogParamsBound = false;

	/** 맵이 열린 직후 뷰포트 크기가 확정되면 플레이어 위치로 스크롤을 보정하기 위한 대기 플래그 */
	bool bPendingCenterOnOpen = false;

	bool bIsDraggingMap = false;

	float CurrentZoom = 1.f;

	/** MapContentSizeBox에 RenderTranslation으로 적용되는 현재 팬(드래그 이동) 오프셋 */
	FVector2D PanOffset = FVector2D::ZeroVector;
	
	FVector2D CurrentContentSize;
};
 
