#include "KOComboComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/KOAbilitySystemComponent.h"
#include "AbilitySystem/Tag/Data/KOGameplayTags_Data.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "AbilitySystem/Tag/Input/KOGameplayTags_Input.h"
#include "UObject/FastReferenceCollector.h"


UKOComboComponent::UKOComboComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UKOComboComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (BufferInfo.BufferInput != EAttackInputType::None)
	{
		BufferInfo.RemainingTime -= DeltaTime;
		
		if (BufferInfo.RemainingTime <= 0.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ComboComponent] 선입력 버퍼 시간 만료."));
			BufferInfo.BufferInput = EAttackInputType::None;
			BufferInfo.RemainingTime = 0.0f;
		}
	}
	
	if (!bIsAttacking && !CurrentComboRowName.IsNone())
	{
		ComboResetTimer -= DeltaTime;
        
		if (ComboResetTimer <= 0.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ComboComponent] 콤보 입력 대기 시간 초과. 타수를 리셋"));
			ResetBuffer(); 
		}
	}
}

void UKOComboComponent::RegisterInput(EAttackInputType InputType)
{
	bool bSuccess = TryExcuteAttack(InputType);
	
	if (bSuccess)
	{
		BufferInfo.BufferInput = EAttackInputType::None;
		BufferInfo.RemainingTime = 0.0f;
		UE_LOG(LogTemp, Warning, TEXT("[ComboComponent] 선입력 공격 실행 -> 버퍼 리셋"));
	}
	else
	{
		BufferInfo.BufferInput = InputType;
		BufferInfo.RemainingTime = MaxBufferGraceTime;
		UE_LOG(LogTemp, Warning, TEXT("[ComboComponent] 동작 중임 실행 대기 입력을 버퍼에 저장"));
	}
}

void UKOComboComponent::ConsumeBuffer()
{
	bIsAttacking = false;
	
	ComboResetTimer = MaxComboResetTime;
	
	if (BufferInfo.BufferInput != EAttackInputType::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ComboComponent] 저장된 버퍼 사용"));
		EAttackInputType SavedInput = BufferInfo.BufferInput;
		BufferInfo.BufferInput = EAttackInputType::None;
		BufferInfo.RemainingTime = 0.0f;
		RegisterInput(SavedInput);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ComboComponent] 노티파이 터졌거나 저장된 버퍼가 없음"));
	}
}

void UKOComboComponent::ResetBuffer()
{
	BufferInfo.BufferInput = EAttackInputType::None;
	BufferInfo.RemainingTime = 0.0f;
	CurrentComboRowName = NAME_None;
	bIsAttacking = false;
	ComboResetTimer = 0.0f;
}

bool UKOComboComponent::TryExcuteAttack(EAttackInputType InputType)
{
	if (bIsAttacking)
	{
		return false;
	}
	
	AController* OwnerController = Cast<AController>(GetOwner());
	if (!OwnerController) return false;

	APawn* ControlledPawn = OwnerController->GetPawn();
    
	if (!ControlledPawn || !ComboDataTable) 
	{
		return false;
	}
	
	FName NextRowName = NAME_None;
	if (CurrentComboRowName.IsNone())
	{
		NextRowName = 
			(InputType == EAttackInputType::Light) ? FName("Light1") : FName("Heavy1");
	}
	else
	{
		FString Context = TEXT("Combo Component Excution");
		FKOComboActionData* CurrentData = ComboDataTable->FindRow<FKOComboActionData>(CurrentComboRowName, Context);
		if (CurrentData)
		{
			NextRowName = 
				(InputType == EAttackInputType::Light) ? CurrentData->NextLightRow : CurrentData->NextHeavyRow;
		}
	}
	
	if (NextRowName.IsNone())
	{
		return false;
	}
	
	FString Context = TEXT("Extract Target Combo Data");
	FKOComboActionData* TargetData = ComboDataTable-> FindRow<FKOComboActionData>(NextRowName, Context);
	
	if (!TargetData || !TargetData->ComboMontage)
	{
		return false;
	}
	
	FGameplayEventData Payload;
	Payload.OptionalObject = TargetData->ComboMontage;
	Payload.OptionalObject2 = TargetData->DamageEffect;
	
	FGameplayTag ExecuteTag;
	if (InputType == EAttackInputType::Light)
	{
		ExecuteTag = FGameplayTag::RequestGameplayTag(FName("Event.Attack.Light.Execute"));
	}
	else
	{
		ExecuteTag = FGameplayTag::RequestGameplayTag(FName("Event.Attack.Heavy.Execute"));
	}
	
	CurrentComboRowName = NextRowName;
	bIsAttacking = true;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(ControlledPawn, ExecuteTag, Payload);
	
	return true;
}

