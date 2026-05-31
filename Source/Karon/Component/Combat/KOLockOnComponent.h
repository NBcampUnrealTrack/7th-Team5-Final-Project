// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "KOLockOnComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockOnChanged, bool, bIsLockedOn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetChanged, AActor*, NewTarget);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KARON_API UKOLockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UKOLockOnComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// GAS Ability에서 호출
	void ActivateLockOn();
	void DeactivateLockOn();
	void SwitchTarget(bool bSwitchRight);

	bool IsLockedOn() const { return bIsLockedOn; }
	AActor* GetLockedTarget() const { return LockedTarget; }
	FVector GetTargetSocketLocation() const;

	UPROPERTY(BlueprintAssignable)
	FOnLockOnChanged OnLockOnChanged;

	UPROPERTY(BlueprintAssignable)
	FOnTargetChanged OnTargetChanged;

	
protected:
	
	virtual void BeginPlay() override;

private:
	// 락온 탐색 반경
	UPROPERTY(EditAnywhere, Category = "LockOn")
	float SearchRadius = 1500.f;

	UPROPERTY(EditAnywhere, Category = "LockOn")
	float CameraInterpSpeed = 8.f;

	// 바라볼 소켓 이름
	UPROPERTY(EditAnywhere, Category = "LockOn")
	FName TargetSocketName = FName("LockOnSocket");

	// 락온 활성화 시 캐릭터에 부여할 GAS Tag
	//UPROPERTY(EditAnywhere, Category = "LockOn")
	//FGameplayTag LockOnActiveTag;

	UPROPERTY()
	AActor* LockedTarget = nullptr;

	bool bIsLockedOn = false;

	AActor* FindBestTarget() const;
	void UpdateCameraRotation(float DeltaTime) const;
	void ApplyLockOnGameplayTag(bool bApply) const;
	bool IsTargetValid() const;
	
	


		
};
