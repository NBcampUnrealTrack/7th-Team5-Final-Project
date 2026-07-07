#include "KOCameraComponent.h"
#include "KOCameraManager.h"

void UKOCameraComponent::TickComponent(
	float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	if (AKOCameraManager* CameraManager = GetOrCacheCameraManager())
	{
		FieldOfView = CameraManager->GetModifiedFOV(DeltaTime, FieldOfView);
	}
	else
	{
		FieldOfView = BaseFOV;
	}
	
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

AKOCameraManager* UKOCameraComponent::GetOrCacheCameraManager()
{
	if (CachedCameraManager) return CachedCameraManager;
	
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return nullptr;
	
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC) return nullptr;

	CachedCameraManager = Cast<AKOCameraManager>(PC->PlayerCameraManager);
	return CachedCameraManager;
}
