#include "KOCameraManager.h"

#include "Character/KOCharacterBase.h"
#include "Modifier/KOCameraModifier.h"

AKOCameraManager::AKOCameraManager()
{
	
}

FRotator AKOCameraManager::GetModifiedControlRotation(float DeltaTime, FRotator BaseRotation) const
{
	FRotator Result = BaseRotation;
	
	for (UKOCameraModifier* Mod : Modifiers)
	{
		if (Mod && Mod->IsModifierActive())
		{
			Mod->ProcessControlRotation(DeltaTime, Result);
		}
	}
	
	return Result;
}

float AKOCameraManager::GetModifiedArmLength(float DeltaTime, float BaseArmLength) const
{
	float Result = BaseArmLength;
	
	for (UKOCameraModifier* Mod : Modifiers)
	{
		if (Mod && Mod->IsModifierActive())
		{
			Mod->ProcessArmLength(DeltaTime, Result);
		}
	}
	
	return Result;
}

float AKOCameraManager::GetModifiedFOV(float DeltaTime, float BaseFOV) const
{
	float Result = BaseFOV;
	
	for (UKOCameraModifier* Mod : Modifiers)
	{
		if (Mod && Mod->IsModifierActive())
		{
			Mod->ProcessFOV(DeltaTime, Result);
		}
	}
	
	return Result;
}

FVector AKOCameraManager::GetModifiedBoomOffset(float DeltaTime, FVector BaseOffset) const
{
	FVector Result = BaseOffset;
	
	for (UKOCameraModifier* Mod : Modifiers)
	{
		if (Mod && Mod->IsModifierActive())
		{
			Mod->ProcessBoomOffset(DeltaTime, Result);
		}
	}
	
	return Result;
}

void AKOCameraManager::InitModifiersWithCharacter(ACharacter* InCharacter)
{
	for (UKOCameraModifier* Mod : Modifiers)
	{
		if (Mod)
		{
			Mod->OwningCharacter = Cast<AKOCharacterBase>(InCharacter);
		}
	}
}

void AKOCameraManager::BeginPlay()
{
	Super::BeginPlay();
	
	Modifiers.Sort([](const TObjectPtr<UKOCameraModifier>& A, const TObjectPtr<UKOCameraModifier>& B) {
		const int32 APriority = A ? A->Priority : 0;
		const int32 BPriority = B ? B->Priority : 0;
		return APriority < BPriority;
	});
}


