#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOBossCH01GimmickPillar.generated.h"

UCLASS()
class KARON_API AKOBossCH01GimmickPillar : public AActor
{
	GENERATED_BODY()
	
public:
	AKOBossCH01GimmickPillar();
	
	/** 기둥을 파괴 상태로 변경한다. 실제 Actor는 Destroy하지 않는다. */
	UFUNCTION(BlueprintCallable, Category = "Boss|Gimmick|Pillar")
	void BreakPillar();

	// 세이브 로드
	void RestoreFromSave(bool bSavedBroken);

	FName GetPillarSaveId() const { return PillarSaveId; }
	bool IsBrokenForSave() const { return bBroken; }
	
protected:
	virtual void BeginPlay() override;
	
	void ApplyBrokenState();
 
protected:
	UPROPERTY(VisibleAnywhere, Category = "Pillar | Component")
	TObjectPtr<UStaticMeshComponent> PillarMesh;
	
	// 세이브 로드
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Pillar|Save")
	FName PillarSaveId = NAME_None;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Pillar|Save")
	bool bBroken = false;
};
