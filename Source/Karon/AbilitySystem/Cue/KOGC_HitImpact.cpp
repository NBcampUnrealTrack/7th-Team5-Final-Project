#include "KOGC_HitImpact.h"
#include "GameFramework/Character.h"
#include "Utility/Log/KOLogManager.h"


AKOGC_HitImpact::AKOGC_HitImpact()
{
	PrimaryActorTick.bCanEverTick = true;
	bAutoDestroyOnRemove = true;
}

void AKOGC_HitImpact::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!bFlashing) return;

	ElapsedTime += DeltaTime;

	const float FlashAlpha = FMath::Clamp(1.0f - (ElapsedTime / FlashDuration), 0.f, 1.f);
	if (FlashMID)
	{
		FlashMID->SetScalarParameterValue(FlashParamName, FlashAlpha);
	}

	const float ShakeAlpha = FMath::Clamp(1.0f - (ElapsedTime / ShakeDuration), 0.f, 1.f);
	const float NoiseZ = FMath::PerlinNoise1D(ElapsedTime * ShakeFrequency) * ShakeAmplitude * ShakeAlpha;

	for (const TWeakObjectPtr<USkeletalMeshComponent>& WeakMesh : AffectedMeshes)
	{
		if (USkeletalMeshComponent* Mesh = WeakMesh.Get())
		{
			if (const FVector* OriginalLocation = OriginalRelativeLocations.Find(WeakMesh))
			{
				Mesh->SetRelativeLocation(*OriginalLocation + FVector(0.f, 0.f, NoiseZ));
			}
		}
	}
}

void AKOGC_HitImpact::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (const TWeakObjectPtr<USkeletalMeshComponent>& WeakMesh : AffectedMeshes)
	{
		if (USkeletalMeshComponent* Mesh = WeakMesh.Get())
		{
			Mesh->SetOverlayMaterial(nullptr);

			if (const FVector* OriginalLocation = OriginalRelativeLocations.Find(WeakMesh))
			{
				Mesh->SetRelativeLocation(*OriginalLocation);
			}
		}
	}	
	
	Super::EndPlay(EndPlayReason);
}

bool AKOGC_HitImpact::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	ACharacter* Character = Cast<ACharacter>(MyTarget);
	if (!Character || !OverlayMaterialBase) return true;
	
	SetLifeSpan(FMath::Max(FlashDuration, ShakeDuration));
	
	AutoDestroyDelay = FMath::Max(FlashDuration, ShakeDuration);
	
	if (!FlashMID)
	{
		FlashMID = UMaterialInstanceDynamic::Create(OverlayMaterialBase, this);
	}
	FlashMID->SetScalarParameterValue(FlashParamName, 1.0f);
	
	TArray<USkeletalMeshComponent*> Meshes;
	Character->GetComponents<USkeletalMeshComponent>(Meshes);

	AffectedMeshes.Reset();
	OriginalRelativeLocations.Reset();

	for (USkeletalMeshComponent* Mesh : Meshes)
	{
		if (!Mesh) continue;

		Mesh->SetOverlayMaterial(FlashMID);

		AffectedMeshes.Add(Mesh);
		OriginalRelativeLocations.Add(Mesh, Mesh->GetRelativeLocation());
	}

	ElapsedTime = 0.f;
	bFlashing = true;

	return true;
}
