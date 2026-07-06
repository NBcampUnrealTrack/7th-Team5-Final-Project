#include "UI/Map/FOW/KOFogManager.h"
#include "UI/Map/FOW/KOVisionComponent.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "TextureResource.h"
#include "Engine/TextureRenderTarget2D.h"
 
AKOFogManager::AKOFogManager()
{
	PrimaryActorTick.bCanEverTick = true;
}
 
void AKOFogManager::BeginPlay()
{
	Super::BeginPlay();
 
	if (CurrentFogRT && ExploredFogRT)
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), CurrentFogRT, FLinearColor::Black);
		UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), ExploredFogRT, FLinearColor::Black);
	}
 
	if (DrawMaterial)
	{
		DrawMID = UMaterialInstanceDynamic::Create(DrawMaterial, this);
	}
	if (CombineMaterial)
	{
		CombineMID = UMaterialInstanceDynamic::Create(CombineMaterial, this);
	}
}
 
void AKOFogManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
 
	AccumulatedTime += DeltaTime;
	if (AccumulatedTime >= UpdateInterval)
	{
		UpdateFog();
		AccumulatedTime = 0.f;
	}
}
 
void AKOFogManager::RegisterVision(UKOVisionComponent* Comp)
{
	if (Comp)
	{
		RegisteredVisionComponents.AddUnique(Comp);
	}
}
 
void AKOFogManager::UnregisterVision(UKOVisionComponent* Comp)
{
	RegisteredVisionComponents.Remove(Comp);
}
 
FVector2D AKOFogManager::WorldToUV(const FVector& WorldPos) const
{
	const float U = (WorldPos.X - MapOrigin.X) / MapSize.X;
	const float V = (WorldPos.Y - MapOrigin.Y) / MapSize.Y;
	return FVector2D(U, V);
}

void AKOFogManager::UpdateFog()
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

bool AKOFogManager::ReadPixelsFromRT(UTextureRenderTarget2D* RT, TArray<FColor>& OutPixels) const
{
	if (!RT || !RT->GetResource())
	{
		return false;
	}
 
	FTextureRenderTargetResource* RTResource = static_cast<FTextureRenderTargetResource*>(RT->GetResource());
	return RTResource->ReadPixels(OutPixels);
}
 
bool AKOFogManager::IsLocationVisible(const FVector& WorldLocation) const
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
 
bool AKOFogManager::IsLocationExplored(const FVector& WorldLocation) const
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
 
