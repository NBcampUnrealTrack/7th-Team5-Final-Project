#include "UI/Map/FOW/KOFogManagerSubsystem.h"
#include "UI/Map/FOW/KOVisionComponent.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "TextureResource.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "PixelFormat.h"

UKOFogManagerSubsystem* UKOFogManagerSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World)
	{
		return nullptr;
	}

	const UGameInstance* GI = World->GetGameInstance();
	ULocalPlayer* LocalPlayer = GI ? GI->GetFirstGamePlayer() : nullptr;
	return LocalPlayer ? LocalPlayer->GetSubsystem<UKOFogManagerSubsystem>() : nullptr;
}

void UKOFogManagerSubsystem::Deinitialize()
{
	bDeinitialized = true;
	RegisteredVisionComponents.Empty();

	Super::Deinitialize();
}

UWorld* UKOFogManagerSubsystem::GetWorld() const
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	return LocalPlayer ? LocalPlayer->GetWorld() : nullptr;
}

void UKOFogManagerSubsystem::RegisterConfig(
	UTextureRenderTarget2D* InCurrentFogRT,
	UTextureRenderTarget2D* InExploredFogRT,
	UMaterialInterface* InDrawMaterial,
	UMaterialInterface* InCombineMaterial,
	const FVector2D& InMapSize,
	const FVector2D& InMapOrigin,
	float InUpdateInterval)
{
	CurrentFogRT = InCurrentFogRT;
	ExploredFogRT = InExploredFogRT;
	DrawMaterial = InDrawMaterial;
	CombineMaterial = InCombineMaterial;
	MapSize = InMapSize;
	MapOrigin = InMapOrigin;
	UpdateInterval = InUpdateInterval;
	AccumulatedTime = 0.f;

	// RegisteredVisionComponents는 여기서 비우지 않는다: 같은 레벨의 Vision 컴포넌트가
	// AKOFogManager보다 먼저 BeginPlay하여 이미 등록을 마쳤을 수 있는데(액터 간 BeginPlay
	// 순서는 보장되지 않음), 여기서 Empty()하면 그 등록이 지워지고 UKOVisionComponent::BeginPlay는
	// 다시 호출되지 않으므로 이후 영원히 시야가 등록되지 않아 안개가 갱신되지 않는다.
	// 유효하지 않은 항목은 UpdateFog()의 루프에서 이미 방어적으로 제거된다.
	CachedVisiblePixels.Empty();
	CachedExploredPixels.Empty();
	CachedSizeX = 0;
	CachedSizeY = 0;

	if (CurrentFogRT && ExploredFogRT)
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), CurrentFogRT, FLinearColor::Black);
		UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), ExploredFogRT, FLinearColor::Black);
	}

	DrawMID = DrawMaterial ? UMaterialInstanceDynamic::Create(DrawMaterial, this) : nullptr;
	CombineMID = CombineMaterial ? UMaterialInstanceDynamic::Create(CombineMaterial, this) : nullptr;

	bConfigured = true;
	
	if (bHasPendingFogLoad)
	{
		ApplyExploredPixelsToRenderTarget(PendingExploredPixels, PendingFogSizeX, PendingFogSizeY);

		PendingExploredPixels.Reset();
		PendingFogSizeX = 0;
		PendingFogSizeY = 0;
		bHasPendingFogLoad = false;
	}
}

bool UKOFogManagerSubsystem::IsTickable() const
{
	return bConfigured && !bDeinitialized;
}

TStatId UKOFogManagerSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UKOFogManagerSubsystem, STATGROUP_Tickables);
}

void UKOFogManagerSubsystem::Tick(float DeltaTime)
{
	AccumulatedTime += DeltaTime;
	if (AccumulatedTime >= UpdateInterval)
	{
		UpdateFog();
		AccumulatedTime = 0.f;
	}
}

void UKOFogManagerSubsystem::RegisterVision(UKOVisionComponent* Comp)
{
	if (Comp)
	{
		RegisteredVisionComponents.AddUnique(Comp);
	}
}

void UKOFogManagerSubsystem::UnregisterVision(UKOVisionComponent* Comp)
{
	RegisteredVisionComponents.Remove(Comp);
}

FVector2D UKOFogManagerSubsystem::WorldToUV(const FVector& WorldPos) const
{
	const float U = (WorldPos.X - MapOrigin.X) / MapSize.X;
	const float V = (WorldPos.Y - MapOrigin.Y) / MapSize.Y;
	return FVector2D(U, V);
}

bool UKOFogManagerSubsystem::GetFogStateForSave(TArray<FColor>& OutExploredPixels, int32& OutSizeX,
	int32& OutSizeY) const
{
	OutExploredPixels.Reset();
	OutSizeX = 0;
	OutSizeY = 0;

	if (!ExploredFogRT)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[FogSave] ExploredFogRT가 없습니다.")
		);

		return false;
	}

	const int32 SizeX = ExploredFogRT->SizeX;
	const int32 SizeY = ExploredFogRT->SizeY;
	const int64 ExpectedPixelCount = static_cast<int64>(SizeX) * static_cast<int64>(SizeY);

	if (SizeX <= 0 || SizeY <= 0)
	{
		return false;
	}

	// 저장하는 순간 Render Target에서 직접 읽는다.
	TArray<FColor> ReadPixels;

	if (ReadPixelsFromRT(ExploredFogRT, ReadPixels) &&
		ReadPixels.Num() == ExpectedPixelCount)
	{
		OutExploredPixels = MoveTemp(ReadPixels);
		OutSizeX = SizeX;
		OutSizeY = SizeY;

		return true;
	}

	// Render Target 읽기에 실패하면 마지막 캐시 데이터를 사용한다.
	if (CachedExploredPixels.Num() == ExpectedPixelCount &&
		CachedSizeX == SizeX &&
		CachedSizeY == SizeY)
	{
		OutExploredPixels = CachedExploredPixels;
		OutSizeX = CachedSizeX;
		OutSizeY = CachedSizeY;

		return true;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"[FogSave] 탐험 안개 픽셀 읽기 실패. "
			"RT=%dx%d Cached=%dx%d CachedPixels=%d"
		),
		SizeX,
		SizeY,
		CachedSizeX,
		CachedSizeY,
		CachedExploredPixels.Num()
	);

	return false;
}

void UKOFogManagerSubsystem::LoadFogStateFromSave(const TArray<FColor>& InExploredPixels, int32 InSizeX, int32 InSizeY)
{
	const int64 ExpectedPixelCount = static_cast<int64>(InSizeX) * static_cast<int64>(InSizeY);

	if (InSizeX <= 0 ||
		InSizeY <= 0 ||
		InExploredPixels.Num() != ExpectedPixelCount)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"[FogLoad] 저장된 안개 데이터가 유효하지 않습니다. "
				"Size=%dx%d Pixels=%d"
			),
			InSizeX,
			InSizeY,
			InExploredPixels.Num()
		);

		return;
	}

	// FogManager 액터의 BeginPlay가 아직 실행되지 않았으면
	// RegisterConfig()가 호출될 때까지 보관한다.
	if (!bConfigured || !ExploredFogRT)
	{
		PendingExploredPixels = InExploredPixels;
		PendingFogSizeX = InSizeX;
		PendingFogSizeY = InSizeY;
		bHasPendingFogLoad = true;

		UE_LOG(
			LogTemp,
			Log,
			TEXT("[FogLoad] Fog 설정 전이므로 복원 데이터를 대기시킵니다.")
		);

		return;
	}

	ApplyExploredPixelsToRenderTarget(InExploredPixels, InSizeX, InSizeY);
}

void UKOFogManagerSubsystem::UpdateFog()
{
	if (!CurrentFogRT || !ExploredFogRT || !DrawMID || !CombineMID)
	{
		return;
	}

	UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), CurrentFogRT, FLinearColor::Black);

	for (int32 i = RegisteredVisionComponents.Num() - 1; i >= 0; --i)
	{
		UKOVisionComponent* VC = RegisteredVisionComponents[i].Get();
		if (!IsValid(VC) || !IsValid(VC->GetOwner()))
		{
			RegisteredVisionComponents.RemoveAt(i);
			continue;
		}

		const FVector RawLocation = VC->GetVisionLocation();
		const FVector2D UV = WorldToUV(RawLocation);
		DrawMID->SetVectorParameterValue(TEXT("VisionPos"), FLinearColor(UV.X, UV.Y, 0, 0));
		DrawMID->SetScalarParameterValue(TEXT("Radius"), VC->VisionRadius / MapSize.X);

		UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), CurrentFogRT, DrawMID);
	}

	CombineMID->SetTextureParameterValue(TEXT("CurrentTex"), CurrentFogRT);
	CombineMID->SetTextureParameterValue(TEXT("PrevTex"), ExploredFogRT);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), ExploredFogRT, CombineMID);

	ReadPixelsFromRT(CurrentFogRT, CachedVisiblePixels);
	ReadPixelsFromRT(ExploredFogRT, CachedExploredPixels);
	CachedSizeX = CurrentFogRT->SizeX;
	CachedSizeY = CurrentFogRT->SizeY;
}

bool UKOFogManagerSubsystem::ReadPixelsFromRT(UTextureRenderTarget2D* RT, TArray<FColor>& OutPixels) const
{
	if (!RT || !RT->GetResource())
	{
		return false;
	}

	FTextureRenderTargetResource* RTResource = static_cast<FTextureRenderTargetResource*>(RT->GetResource());
	return RTResource->ReadPixels(OutPixels);
}

bool UKOFogManagerSubsystem::ApplyExploredPixelsToRenderTarget(const TArray<FColor>& InExploredPixels, int32 InSizeX,
	int32 InSizeY)
{
	if (!ExploredFogRT)
	{
		return false;
	}

	const int64 ExpectedPixelCount = static_cast<int64>(InSizeX) * static_cast<int64>(InSizeY);

	if (InSizeX <= 0 || InSizeY <= 0 || InExploredPixels.Num() != ExpectedPixelCount)
	{
		return false;
	}

	// 저장 당시 RT 크기와 현재 RT 크기가 다르면 그대로 복원할 수 없다.
	if (ExploredFogRT->SizeX != InSizeX || ExploredFogRT->SizeY != InSizeY)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"[FogLoad] RenderTarget 크기가 다릅니다. "
				"Saved=%dx%d Current=%dx%d"
			),
			InSizeX,
			InSizeY,
			ExploredFogRT->SizeX,
			ExploredFogRT->SizeY
		);

		return false;
	}

	FogRestoreTexture = UTexture2D::CreateTransient(
		InSizeX,
		InSizeY,
		PF_B8G8R8A8,
		TEXT("KO_FogRestoreTexture")
	);

	if (!FogRestoreTexture ||
		!FogRestoreTexture->GetPlatformData() ||
		FogRestoreTexture->GetPlatformData()->Mips.IsEmpty())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[FogLoad] 복원용 Texture2D 생성에 실패했습니다.")
		);

		return false;
	}

	FogRestoreTexture->SRGB = false;
	FogRestoreTexture->NeverStream = true;

	FTexture2DMipMap& Mip = FogRestoreTexture->GetPlatformData()->Mips[0];

	void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);

	if (!TextureData)
	{
		Mip.BulkData.Unlock();
		return false;
	}

	FMemory::Memcpy(
		TextureData,
		InExploredPixels.GetData(),
		InExploredPixels.Num() * sizeof(FColor)
	);

	Mip.BulkData.Unlock();
	FogRestoreTexture->UpdateResource();

	UCanvas* Canvas = nullptr;
	FVector2D CanvasSize = FVector2D::ZeroVector;
	FDrawToRenderTargetContext RenderContext;

	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(
		GetWorld(),
		ExploredFogRT,
		Canvas,
		CanvasSize,
		RenderContext
	);

	if (!Canvas)
	{
		UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(GetWorld(), RenderContext);

		return false;
	}

	Canvas->K2_DrawTexture(
		FogRestoreTexture,
		FVector2D::ZeroVector,
		CanvasSize,
		FVector2D::ZeroVector,
		FVector2D(1.f, 1.f),
		FLinearColor::White,
		BLEND_Opaque,
		0.f,
		FVector2D::ZeroVector
	);

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(GetWorld(), RenderContext);

	// 위치 탐색 함수도 복원된 데이터를 즉시 사용하도록 캐시 갱신
	CachedExploredPixels = InExploredPixels;
	CachedSizeX = InSizeX;
	CachedSizeY = InSizeY;

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"[FogLoad] 탐험 안개 복원 완료. Size=%dx%d Pixels=%d"
		),
		InSizeX,
		InSizeY,
		InExploredPixels.Num()
	);

	return true;
}

bool UKOFogManagerSubsystem::IsLocationVisible(const FVector& WorldLocation) const
{
	if (CachedVisiblePixels.Num() == 0)
	{
		return true;
	}

	const FVector2D UV = WorldToUV(WorldLocation);
	if (UV.X < 0.f || UV.X > 1.f || UV.Y < 0.f || UV.Y > 1.f)
	{
		return false;
	}

	const int32 PX = FMath::Clamp((int32)(UV.X * CachedSizeX), 0, CachedSizeX - 1);
	const int32 PY = FMath::Clamp((int32)(UV.Y * CachedSizeY), 0, CachedSizeY - 1);
	const int32 Idx = PY * CachedSizeX + PX;

	return CachedVisiblePixels.IsValidIndex(Idx) && CachedVisiblePixels[Idx].R > 25;
}

bool UKOFogManagerSubsystem::IsLocationExplored(const FVector& WorldLocation) const
{
	if (CachedExploredPixels.Num() == 0)
	{
		return true;
	}

	const FVector2D UV = WorldToUV(WorldLocation);
	if (UV.X < 0.f || UV.X > 1.f || UV.Y < 0.f || UV.Y > 1.f)
	{
		return false;
	}

	const int32 PX = FMath::Clamp((int32)(UV.X * CachedSizeX), 0, CachedSizeX - 1);
	const int32 PY = FMath::Clamp((int32)(UV.Y * CachedSizeY), 0, CachedSizeY - 1);
	const int32 Idx = PY * CachedSizeX + PX;

	return CachedExploredPixels.IsValidIndex(Idx) && CachedExploredPixels[Idx].R > 25;
}