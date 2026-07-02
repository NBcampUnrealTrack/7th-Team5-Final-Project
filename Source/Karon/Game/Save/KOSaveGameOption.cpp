#include "KOSaveGameOption.h"
#include "Kismet/GameplayStatics.h"

const FString UKOSaveGameOption::SlotName = TEXT("OptionSave");

UKOSaveGameOption* UKOSaveGameOption::LoadOrCreate(UObject* WorldContextObject)
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		UKOSaveGameOption* Loaded = Cast<UKOSaveGameOption>(
			UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
		if (Loaded)
		{
			return Loaded;
		}
	}

	return Cast<UKOSaveGameOption>(
		UGameplayStatics::CreateSaveGameObject(UKOSaveGameOption::StaticClass()));
}

bool UKOSaveGameOption::Save(UObject* WorldContextObject, UKOSaveGameOption* Option)
{
	if (!Option)
	{
		return false;
	}
	return UGameplayStatics::SaveGameToSlot(Option, SlotName, UserIndex);
}