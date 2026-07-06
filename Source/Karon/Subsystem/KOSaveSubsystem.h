#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "KOSaveSubsystem.generated.h"

class AKOPlayerController;
class UKOInventoryComponent;
class UKOEquipmentComponent;
class UKOBuildUIComponent;
class UKOSkillSubsystem;

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
    
    UFUNCTION(BlueprintCallable, Category = "SaveLoad|Combat")
    void NotifyActorTargetingPlayer(AActor* SourceActor);

    UFUNCTION(BlueprintCallable, Category = "SaveLoad|Combat")
    void NotifyActorStoppedTargetingPlayer(AActor* SourceActor);

    UFUNCTION(BlueprintCallable, Category = "SaveLoad|Combat")
    bool CanSaveOrLoad() const;
    
    // 채집한 채집물 목록
    void MarkItemDropCollected(FName DropSaveId);
    
    // 죽은 몬스터 목록
    void MarkMonsterDead(FName MonsterSaveId);

private:
    static const FString DefaultSlotName;
    static constexpr int32 DefaultUserIndex = 0;

    AKOPlayerController* GetKOPlayerController() const;
    UKOInventoryComponent* GetPlayerInventory(AKOPlayerController* PC) const;
    UKOEquipmentComponent* GetPlayerEquipment(AKOPlayerController* PC) const;
    UKOBuildUIComponent* GetPlayerBuildUI(AKOPlayerController* PC) const;
    UKOSkillSubsystem* GetPlayerSkillSubsystem(AKOPlayerController* PC) const;
    
private:
    UPROPERTY()
    TSet<TWeakObjectPtr<AActor>> ActorsTargetingPlayer;

    UPROPERTY(EditDefaultsOnly, Category = "SaveLoad|Combat")
    float SaveLoadUnlockDelayAfterCombat = 5.0f;

    FTimerHandle SaveLoadUnlockTimerHandle;

    bool bSaveLoadBlockedByCombat = false;
    
    // 채집물
    TSet<FName> CollectedItemDropIds;
    // 몬스터
    TSet<FName> DeadMonsterIds;
};