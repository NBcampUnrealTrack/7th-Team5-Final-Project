#pragma once
 
#include "CoreMinimal.h"
#include "Character/KOCharacterBase.h"
#include "KOBossBase.generated.h"
 
struct FStreamableHandle;
class UKOBossDataAsset;
class UStreamableHandle;
 
DECLARE_MULTICAST_DELEGATE(FOnBossReady);
DECLARE_MULTICAST_DELEGATE(FOnBossDetectedPlayer);
DECLARE_MULTICAST_DELEGATE(FOnBossDeathAnimEnd);
 
UCLASS()
class KARON_API AKOBossBase : public AKOCharacterBase
{
	GENERATED_BODY()
 
public:
	AKOBossBase(const FObjectInitializer& ObjectInitializer);

	// 외부 호출
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void StartAsyncLoad(UKOBossDataAsset* InDataAsset);
 
	// 델리게이트
	FOnBossReady OnBossReady;
	FOnBossDetectedPlayer OnBossDetectedPlayer;
	FOnBossDeathAnimEnd OnBossDeathAnimEnd;
	
	UKOBossDataAsset* GetDataAsset() const { return DataAsset; }
	
	void NotifyPlayerDetected();
	void NotifyDeathAnimEnd();
 
protected:
	virtual void BeginPlay() override;
	
	virtual void OnBossInitialized() {}
	
	virtual void OnPhaseChanged(int32 NewPhase) {}
	
	virtual void OnGroggyBegin() {}
	virtual void OnGroggyEnd() {}
	
	virtual void OnBossDeath() {}
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attribute | Combat")
	TObjectPtr<UKOCombatSet> CombatSet;
 
	UPROPERTY()
	TObjectPtr<UKOBossDataAsset> DataAsset;
	
	// 비동기 로드 테스트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTest")
	TObjectPtr<UKOBossDataAsset> DefaultDataAsset;
 
private:
	TSharedPtr<FStreamableHandle> StreamableHandle;
 
	void OnAssetsLoaded();
	void ApplyMeshAndAnim();
	void ApplyStats();
	void ApplyAbilities();
	
	bool bPlayerDetected = false;
 
	// 페이즈 관리
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PhaseRatio = 0.5f;
 
private:
	bool bPhase2Triggered = false;
 
	// 델리게이트 콜백 함수
	UFUNCTION()
	void OnHealthChangedCallback(float OldVal, float NewVal);
};
