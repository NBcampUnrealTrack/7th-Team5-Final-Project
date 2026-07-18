#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/Boss/KOBossBase.h"
#include "Components/PointLightComponent.h"
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
	
	UPROPERTY(VisibleAnywhere, Category = "Boss|FaceLight")
	TObjectPtr<UPointLightComponent> FaceLight;

	// 라이트를 부착할 소켓 이름
	UPROPERTY(EditAnywhere, Category = "Boss|FaceLight")
	FName FaceLightSocket = FName("head");

	// 페이즈 1 색상
	UPROPERTY(EditAnywhere, Category = "Boss|FaceLight")
	FLinearColor FaceLightColorNormal = FLinearColor(1.f, 0.8f, 0.f);

	// 페이즈 2 색상
	UPROPERTY(EditAnywhere, Category = "Boss|FaceLight")
	FLinearColor FaceLightColorPhase2 = FLinearColor(1.f, 0.1f, 0.f);

	UPROPERTY(EditAnywhere, Category = "Boss|FaceLight")
	float FaceLightIntensity = 3000.f;

	UPROPERTY(EditAnywhere, Category = "Boss|FaceLight")
	float FaceLightRadius = 300.f;
};
