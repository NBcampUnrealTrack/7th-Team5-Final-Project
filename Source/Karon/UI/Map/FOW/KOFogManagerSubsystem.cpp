#include "UI/Map/FOW/KOFogManagerSubsystem.h"
#include "UI/Map/FOW/KOVisionComponent.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "TextureResource.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"

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