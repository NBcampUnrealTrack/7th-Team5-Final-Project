#include "KOSaveSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Game/Save/KOSaveGame.h"
#include "Game/KOPlayerController.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "Component/Inventory/KOEquipmentComponent.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Data/Equipment/KOWeaponDefinition.h"

const FString UKOSaveSubsystem::DefaultSlotName = TEXT("KaronSaveSlot");

UKOSaveSubsystem* UKOSaveSubsystem::Get(const UObject* WorldContext)
{
	if (!WorldContext || !GEngine)
	{
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(
		WorldContext,
		EGetWorldErrorMode::LogAndReturnNull
	);

	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UKOSaveSubsystem>() : nullptr;
}

AKOPlayerController* UKOSaveSubsystem::GetKOPlayerController() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	return Cast<AKOPlayerController>(UGameplayStatics::GetPlayerController(World, 0));
}

UKOInventoryComponent* UKOSaveSubsystem::GetPlayerInventory(AKOPlayerController* PC) const
{
	if (!PC)
	{
		return nullptr;
	}

	// 네 구조에서는 InventoryComponent가 PlayerController에 붙어 있음
	return PC->FindComponentByClass<UKOInventoryComponent>();
}

UKOEquipmentComponent* UKOSaveSubsystem::GetPlayerEquipment(AKOPlayerController* PC) const
{
	if (!PC)
	{
		return nullptr;
	}

	APawn* Pawn = PC->GetPawn();
	if (!Pawn)
	{
		return nullptr;
	}

	// EquipmentComponent는 캐릭터/Pawn에 붙어 있음
	return Pawn->FindComponentByClass<UKOEquipmentComponent>();
}

bool UKOSaveSubsystem::SaveCurrentGame()
{
	UKOSaveGame* SaveData = Cast<UKOSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UKOSaveGame::StaticClass())
	);

	if (!SaveData)
	{
		return false;
	}

	AKOPlayerController* PC = GetKOPlayerController();
	if (!PC)
	{
		return false;
	}

	// 플레이어 위치 저장
	if (APawn* Pawn = PC->GetPawn())
	{
		SaveData->bHasPlayerTransform = true;
		SaveData->PlayerTransform = Pawn->GetActorTransform();
	}

	// 인벤토리 저장
	if (UKOInventoryComponent* Inventory = GetPlayerInventory(PC))
	{
		SaveData->InventorySlots = Inventory->GetSlots();
	}

	// 장착 무기 저장
	if (UKOEquipmentComponent* Equipment = GetPlayerEquipment(PC))
	{
		const FName WeaponItemId = Equipment->GetCurrentWeaponItemId();

		if (!WeaponItemId.IsNone())
		{
			SaveData->bHasEquippedWeapon = true;
			SaveData->EquippedWeaponItemId = WeaponItemId;
			SaveData->EquippedWeaponSlot = Equipment->GetCurrentWeaponSlot();
		}
		else
		{
			SaveData->bHasEquippedWeapon = false;
			SaveData->EquippedWeaponItemId = NAME_None;
			SaveData->EquippedWeaponSlot = EWeaponSlot::Holster;
		}
	}

	return UGameplayStatics::SaveGameToSlot(
		SaveData,
		DefaultSlotName,
		DefaultUserIndex
	);
}

bool UKOSaveSubsystem::LoadCurrentGame()
{
	if (!DoesSaveExist())
	{
		return false;
	}

	UKOSaveGame* SaveData = Cast<UKOSaveGame>(
		UGameplayStatics::LoadGameFromSlot(DefaultSlotName, DefaultUserIndex)
	);

	if (!SaveData)
	{
		return false;
	}

	AKOPlayerController* PC = GetKOPlayerController();
	if (!PC)
	{
		return false;
	}

	// 플레이어 위치 로드
	if (SaveData->bHasPlayerTransform)
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			Pawn->SetActorTransform(SaveData->PlayerTransform);
		}
	}

	// 인벤토리 로드
	if (UKOInventoryComponent* Inventory = GetPlayerInventory(PC))
	{
		Inventory->LoadSlotsFromSave(SaveData->InventorySlots);
	}

	// 장착 무기 로드
	if (UKOEquipmentComponent* Equipment = GetPlayerEquipment(PC))
	{
		if (!SaveData->bHasEquippedWeapon || SaveData->EquippedWeaponItemId.IsNone())
		{
			Equipment->UnequipWeapon();
		}
		else
		{
			UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(PC);
			if (!LoadSub)
			{
				Equipment->UnequipWeapon();
				return true;
			}

			UKOWeaponDefinition* WeaponDef =
				LoadSub->ResolveWeaponDefinitionByItemId(SaveData->EquippedWeaponItemId);

			if (!WeaponDef)
			{
				UE_LOG(
					LogTemp,
					Warning,
					TEXT("[SaveLoad] WeaponDefinition 로드 실패: ItemId=%s"),
					*SaveData->EquippedWeaponItemId.ToString()
				);

				Equipment->UnequipWeapon();
				return true;
			}

			Equipment->RestoreWeaponFromSave(
				SaveData->EquippedWeaponItemId,
				WeaponDef,
				SaveData->EquippedWeaponSlot
			);
		}
	}

	return true;
}

bool UKOSaveSubsystem::DoesSaveExist() const
{
	return UGameplayStatics::DoesSaveGameExist(
		DefaultSlotName,
		DefaultUserIndex
	);
}

bool UKOSaveSubsystem::DeleteSave()
{
	if (!DoesSaveExist())
	{
		return false;
	}

	return UGameplayStatics::DeleteGameInSlot(
		DefaultSlotName,
		DefaultUserIndex
	);
}