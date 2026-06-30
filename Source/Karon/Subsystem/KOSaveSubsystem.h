#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "KOSaveSubsystem.generated.h"

class AKOPlayerController;
class UKOInventoryComponent;
class UKOEquipmentComponent;

UCLASS()
class KARON_API UKOSaveSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    static UKOSaveSubsystem* Get(const UObject* WorldContext);

    UFUNCTION(BlueprintCallable, Category = "KO|Save")
    bool SaveCurrentGame();

    UFUNCTION(BlueprintCallable, Category = "KO|Save")
    bool LoadCurrentGame();

    UFUNCTION(BlueprintCallable, Category = "KO|Save")
    bool DoesSaveExist() const;

    UFUNCTION(BlueprintCallable, Category = "KO|Save")
    bool DeleteSave();

private:
    static const FString DefaultSlotName;
    static constexpr int32 DefaultUserIndex = 0;

    AKOPlayerController* GetKOPlayerController() const;
    UKOInventoryComponent* GetPlayerInventory(AKOPlayerController* PC) const;
    UKOEquipmentComponent* GetPlayerEquipment(AKOPlayerController* PC) const;
};