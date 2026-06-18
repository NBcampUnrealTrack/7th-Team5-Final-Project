#pragma once

#include "CoreMinimal.h"
#include "KOGA_AttackBase.h"
#include "KOGA_Attack_Light.generated.h"

class UGameplayEffect;
class UAbilityTask_PlayMontageAndWait;

UCLASS()
class KARON_API UKOGA_Attack_Light : public UKOGA_AttackBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Attack_Light();
	
protected:
	// 수정 1 :  CanAbility 의미가 없음. (단순 캐릭터인지 확인만) 
    	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void InputPressed(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
	) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Data")
	TObjectPtr<UDataTable> ComboDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Data")
	FName WeaponRowName = FName("DefaultWeapon");
	
	// (캐릭터 -> EquipmentComp)-> WeaponData-> ? 
	
	// 콤보 데이터 테이블 - 무기 Def 통합 
	
private:
	int32 CurrentComboIndex;
	int32 MaxComboCount;
	bool bIsComboQueued;
	bool bIsInputBufferOpen;
	
	UPROPERTY()
	TObjectPtr<UAnimMontage> ComboMontage;
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> CurrentMontageTask;
	
	UFUNCTION()
	void PlayNextComboSection();
	
	UFUNCTION()
	void OnComboWindowReceived(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnMontageEnded();
	
	UFUNCTION()
	void OnHitEventReceived(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnInputBufferOpened(FGameplayEventData Payload);
};
