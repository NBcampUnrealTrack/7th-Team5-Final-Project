#include "KOSpringArmComponent.h"
#include "KOCameraManager.h"

void UKOSpringArmComponent::TickComponent(
	float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	if (AKOCameraManager* CameraManager = GetOrCacheCameraManager())
	{
		TargetArmLength = CameraManager->GetModifiedArmLength(DeltaTime, TargetArmLength);
		SocketOffset = CameraManager->GetModifiedBoomOffset(DeltaTime, SocketOffset);
	}
	else
	{
		TargetArmLength = BaseArmLength;
		SocketOffset = BaseSocketOffset;
	}
	
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

AKOCameraManager* UKOSpringArmComponent::GetOrCacheCameraManager()
{
	if (CachedCameraManager) return CachedCameraManager;
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return nullptr;

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC) return nullptr;

	CachedCameraManager = Cast<AKOCameraManager>(PC->PlayerCameraManager);
	return CachedCameraManager;
}
