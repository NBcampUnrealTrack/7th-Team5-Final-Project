#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/Boss/KOBossBase.h"
#include "KOBossChapter01.generated.h"

UCLASS()
class KARON_API AKOBossChapter01 : public AKOBossBase
{
	GENERATED_BODY()
 
public:
	AKOBossChapter01(const FObjectInitializer& ObjectInitializer);
 
protected:
	virtual void BeginPlay() override;
 
	virtual void OnBossInitialized() override;
	virtual void OnPhaseChanged(int32 NewPhase) override;
	
	virtual void OnGroggyBegin() override;
	virtual void OnGroggyEnd() override;
	virtual void TriggerGroggy() override;
	
	virtual void NotifyGimmickDashEnd() override;
 
	virtual void OnBossDeath() override;
 
	virtual void OnCharacterDead(AActor* DeathInstigator) override;
 
private:
	UPROPERTY(EditAnywhere, Category = "Boss|Core")
	int32 CoreMaterialIndex = 0;
 
	UPROPERTY(EditAnywhere, Category = "Boss|Core")
	float CoreEmissiveIntensity = 5.f;
 
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CoreMID;
 
	bool bCoreOpen = false;
 
	void OpenCore();
	void CloseCore();
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Boss|State")
	bool bIsGroggy = false;
};
