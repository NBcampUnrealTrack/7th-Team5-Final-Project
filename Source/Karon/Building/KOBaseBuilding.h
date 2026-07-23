#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utility/Interface/KOInteractableInterface.h"

#include "KOBaseBuilding.generated.h"

struct FKOFactoryRow;
class UWidgetComponent;
class UKOFactoryProcessorComponent;
class UAudioComponent;
class USoundBase;

UCLASS()
class KARON_API AKOBaseBuilding : public AActor, public IKOInteractableInterface
{
	GENERATED_BODY()

public:
	AKOBaseBuilding();
	
	// 건설 직후 BuildComponent가 FactoryId를 넘겨주는 함수
	void InitializeBuildingData(FName InFactoryId);

	FName GetFactoryId() const { return FactoryId; }

	/** LoadSubsystem을 통해 자신이 참조하는 Factory Row를 조회 */
	const FKOFactoryRow* GetFactoryRow() const;
	
	void SetPressureWarningSuppressed(bool bSuppressed);

	// ─── IKOInteractableInterface ─────────────────────────────────────────────
	virtual bool  CanInteract(AActor* Interactor) const override;
	virtual void  OnInteract(AActor* Interactor) override;
	virtual FText GetInteractionPrompt() const override;
	
	/** 실제 설비 작동 여부에 따라 반복 사운드를 켜거나 끈다. */
	UFUNCTION(BlueprintCallable, Category = "Building|Sound")
	void SetOperatingSoundActive(bool bActive);

	UFUNCTION(BlueprintPure, Category = "Building|Sound")
	bool IsOperatingSoundActive() const;
	
	UFUNCTION(BlueprintCallable, Category = "Building|Sound")
	void PlayOperationBlockedSound();
	
	/** 지정 시간 동안 가동음을 막는다. 설치 사운드와의 중첩 방지용 */
	void BlockOperatingSound(float Duration);

	/** FadeOut 없이 가동음을 즉시 정지한다. 해제 사운드와의 중첩 방지용 */
	void StopOperatingSoundImmediately();
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	// 거리 안일 시 설비 경고 위젯 갱신
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KO|Optimization")
	float WarningWidgetActivationDistance = 1000.f;

	// 위젯 활성 거리 안에 있는지 확인
	bool IsPlayerWithinWarningWidgetDistance() const;
	
	bool bWarningWidgetRangeActive = false;
	bool IsWarningWidgetRangeActive() const { return bWarningWidgetRangeActive; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KO|Energy")
	bool bShowPressureWarning = true; // 압력 부족 표시 사용 여부
	
	bool bPressureWarningSuppressed = false; // 숨김 여부

	// 실제 월드에 설치된 건물이 참조할 Factory DataTable의 RowName
	UPROPERTY(VisibleInstanceOnly, Category = "Building")
	FName FactoryId = NAME_None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building|Sound")
	TObjectPtr<USoundBase> OperatingSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building|Sound", meta = (ClampMin = "0.0"))
	float OperatingSoundVolume = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building|Sound", meta = (ClampMin = "0.01"))
	float OperatingSoundPitch = 1.0f;

	/** 급격하게 끊기지 않도록 사운드가 켜지는 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building|Sound", meta = (ClampMin = "0.0"))
	float OperatingSoundFadeInTime = 0.15f;

	/** 급격하게 끊기지 않도록 사운드가 꺼지는 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building|Sound", meta = (ClampMin = "0.0"))
	float OperatingSoundFadeOutTime = 0.15f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building|Sound")
	TObjectPtr<UAudioComponent> OperatingAudioComponent;
	
	/** 압력 부족 사운드 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building|Sound|Pressure")
	TObjectPtr<USoundBase> PressureShortageSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building|Sound|Pressure", meta = (ClampMin = "0.0"))
	float PressureShortageSoundVolume = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building|Sound|Pressure", meta = (ClampMin = "0.01"))
	float PressureShortageSoundPitch = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building|Sound|Pressure", meta = (ClampMin = "0.0"))
	float PressureShortageSoundStartTime = 0.0f;

private:
	UPROPERTY()
	TObjectPtr<UWidgetComponent> PressureWarningWidget = nullptr;

	UPROPERTY()
	TObjectPtr<UKOFactoryProcessorComponent> CachedProcessor = nullptr;

	void RefreshPressureWarning(); // 표시, 숨김
	bool IsPressureAvailable() const; // 압력 체크
	
	void UpdatePressureWarningFacingCamera();
	
	/** Processor/Producer가 현재 가동음을 요청하고 있는지 */
	bool bOperatingSoundRequested = false;

	/** 설치 사운드 재생 중이라 가동음을 막고 있는지 */
	bool bOperatingSoundBlocked = false;

	FTimerHandle OperatingSoundBlockTimer;

	void ReleaseOperatingSoundBlock();
	void StartOperatingSoundIfAllowed();
};
