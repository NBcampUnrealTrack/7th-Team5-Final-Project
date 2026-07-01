#include "KOGC_OverClock_Aura.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/GameplayStatics.h"


AKOGC_OverClock_Aura::AKOGC_OverClock_Aura()
{
	PrimaryActorTick.bCanEverTick = true;

	bAutoDestroyOnRemove = false;
	
	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
	RootComponent = PostProcessComponent;

	CurrentAlpha = 0.0f;
	TargetAlpha = 0.0f;
	bIsFadingOut = false;
	
	SlowMotionScale = 0.2f;              
	SlowMotionRealTimeDuration = 0.2f;
}

void AKOGC_OverClock_Aura::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!OverclockMID)
	{
		return;
	}

	CurrentAlpha = FMath::FInterpTo(CurrentAlpha, TargetAlpha, DeltaTime, 4.0f);

	OverclockMID->SetScalarParameterValue(FName("VignetteRadius"), CurrentAlpha * 2.0f);
	OverclockMID->SetScalarParameterValue(FName("DistortionIntensity"), CurrentAlpha * 0.05f);

	if (bIsFadingOut && CurrentAlpha <= 0.01f)
	{
		PostProcessComponent->Settings.RemoveBlendable(OverclockMID);

		Destroy();
	}
}

bool AKOGC_OverClock_Aura::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	Super::OnActive_Implementation(MyTarget, Parameters);
	
	if (VignetteMaterialSource)
	{
		if (!OverclockMID)
		{
			OverclockMID = UMaterialInstanceDynamic::Create(VignetteMaterialSource, this);
		}
		
		if (OverclockMID)
		{
			PostProcessComponent->AddOrUpdateBlendable(OverclockMID, 1.0f);
		}
	}
	
	TargetAlpha = 1.0f;
	bIsFadingOut = false;

	if (GetWorld())
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), SlowMotionScale);

		GetWorld()->GetTimerManager().SetTimer(
			TimeDilationTimerHandle,
			this,
			&AKOGC_OverClock_Aura::RestoreTimeDilation,
			SlowMotionScale,
			false
		);
	}
	
	return true;
}

void AKOGC_OverClock_Aura::RestoreTimeDilation()
{
	if (GetWorld())
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
	}
}

bool AKOGC_OverClock_Aura::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	Super::OnRemove_Implementation(MyTarget, Parameters);
	
	TargetAlpha = 0.0f;
	bIsFadingOut = true;
	
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimeDilationTimerHandle);
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
	}
	
	return true;
}

