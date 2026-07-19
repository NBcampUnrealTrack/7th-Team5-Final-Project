#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "KOSaveSubsystem.generated.h"

class AKOPlayerController;
class UKOInventoryComponent;
class UKOEquipmentComponent;
class UKOBuildUIComponent;
class UKOSkillSubsystem;
class UKOQuestGuideSubsystem;

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
    
    // 로드 여부
    UFUNCTION(BlueprintCallable, Category = "KO|Save")
    void RequestLobbyLoad();

    bool ConsumeLobbyLoadRequest();
    
    UFUNCTION(BlueprintCallable, Category = "SaveLoad|Combat")
    void NotifyActorTargetingPlayer(AActor* SourceActor);

    UFUNCTION(BlueprintCallable, Category = "SaveLoad|Combat")
    void NotifyActorStoppedTargetingPlayer(AActor* SourceActor);

    UFUNCTION(BlueprintCallable, Category = "SaveLoad|Combat")
    bool CanSaveOrLoad() const;
    
    UFUNCTION(BlueprintCallable, Category = "SaveLoad|Combat")
    void ForceEndCombat();
    
    // 채집한 채집물 목록
    void MarkItemDropCollected(FName DropSaveId);
    
    // 죽은 몬스터 목록
    void MarkMonsterDead(FName MonsterSaveId);
    
    
    
    // 파괴된 액터 목록
    UFUNCTION(BlueprintCallable)
    void MarkDestoryedActor(FName ActorSaveId);
    
    UFUNCTION(BlueprintCallable)
    bool CheckIsDestroyedActor(FName ActorSaveId);
 
    
    UFUNCTION(BlueprintPure, Category = "KO|Save")
    bool HasLobbyLoadRequest() const { return bLobbyLoadRequested; }

private:
    static const FString DefaultSlotName;
    static constexpr int32 DefaultUserIndex = 0;

    AKOPlayerController* GetKOPlayerController() const;
    UKOInventoryComponent* GetPlayerInventory(AKOPlayerController* PC) const;
    UKOEquipmentComponent* GetPlayerEquipment(AKOPlayerController* PC) const;
    UKOBuildUIComponent* GetPlayerBuildUI(AKOPlayerController* PC) const;
    UKOSkillSubsystem* GetPlayerSkillSubsystem(AKOPlayerController* PC) const;
    UKOQuestGuideSubsystem* GetQuestGuideSubsystem() const;
    
private:
    UPROPERTY()
    TSet<TWeakObjectPtr<AActor>> ActorsTargetingPlayer;

    UPROPERTY(EditDefaultsOnly, Category = "SaveLoad|Combat")
    float SaveLoadUnlockDelayAfterCombat = 5.0f;
    
    double CombatUnlockRealTimeSeconds = 0.0;
    
    bool bLobbyLoadRequested = false;
    
    // 채집물
    TSet<FName> CollectedItemDropIds;
    // 몬스터
    TSet<FName> DeadMonsterIds;
    // 액터
    TSet<FName> DestroyedActorIds;
};