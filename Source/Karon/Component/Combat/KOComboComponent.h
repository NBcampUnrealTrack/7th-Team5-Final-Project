#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/KOComboActionData.h"
#include "GameplayTagContainer.h"
#include "KOComboComponent.generated.h"

USTRUCT(BlueprintType)
struct FInputBufferInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category="Combo Buffer")
	EAttackInputType BufferInput = EAttackInputType::None;
	
	UPROPERTY(BlueprintReadWrite, Category="Combo Buffer")
	float RemainingTime = 0.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KARON_API UKOComboComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKOComboComponent();
	
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
	    FActorComponentTickFunction* ThisTickFunction
	) override;
	
	UFUNCTION(BlueprintCallable, Category = "Combo System")
	void RegisterInput(EAttackInputType InputType);
	
	UFUNCTION(BlueprintCallable, Category = "Combo System")
	void ConsumeBuffer();
	
	UFUNCTION(BlueprintCallable, Category = "Combo System")
	void ResetBuffer();
	
protected:
	UPROPERTY(BlueprintReadWrite, Category = "Combo System|State")
	FInputBufferInfo BufferInfo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo System|Settings")
	float MaxBufferGraceTime = 0.5f;
	
	UPROPERTY(EditDefaultsOnly, Category="Combo System|Data")
	TObjectPtr<UDataTable> ComboDataTable;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combo System|Data")
	FName CurrentComboRowName = NAME_None;
	
private:
	bool TryExcuteAttack(EAttackInputType InputType);
};
