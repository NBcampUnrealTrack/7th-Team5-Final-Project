#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/Boss/KOBossBase.h"
#include "KOBossChapter01.generated.h"

UCLASS()
class KARON_API AKOBossChapter01 : public AKOBossBase
{
	GENERATED_BODY()
 
public:
	AKOBossChapter01();
 
protected:
	virtual void BeginPlay() override;
	
	virtual void OnBossInitialized() override;
	virtual void OnPhaseChanged(int32 NewPhase) override;
	virtual void OnGroggyBegin() override;
	virtual void OnGroggyEnd() override;
	virtual void OnBossDeath() override;
 
private:
	// 코어
	// BP에서 머티리얼 슬롯 인덱스 설정
	UPROPERTY(EditAnywhere, Category = "Boss|Core")
	int32 CoreMaterialIndex = 0;
 
	// 코어 개방 시 발광 강도
	UPROPERTY(EditAnywhere, Category = "Boss|Core")
	float CoreEmissiveIntensity = 5.f;
 
	// 코어 개방 유지 시간
	UPROPERTY(EditAnywhere, Category = "Boss|Core")
	float CoreOpenDuration = 4.f;
	
	// 상태
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category = "Boss|State")
	bool bIsDead = false;
 
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CoreMID;
 
	bool bCoreOpen = false;
 
	void OpenCore();
	void CloseCore();
	
	// 타이머 
	FTimerHandle CoreCloseTimerHandle;
};
