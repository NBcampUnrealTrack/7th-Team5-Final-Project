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
			ResetBuffer();
		}
	}
}

void UKOComboComponent::RegisterInput(EAttackInputType InputType)
{
	bool bSuccess = TryExcuteAttack(InputType);
	
	if (bSuccess)
	{
		ResetBuffer();
		UE_LOG(LogTemp, Warning, TEXT("[ComboComponent] 선입력 공격 실행 -> 버퍼 리셋"));
	}
	else
	{
		BufferInfo.BufferInput = InputType;
		BufferInfo.RemainingTime = MaxBufferGraceTime;
		UE_LOG(LogTemp, Warning, TEXT("[ComboComponent] 동작 중으로 인한 취소 -> 버퍼 리셋"));
	}
}

void UKOComboComponent::ConsumeBuffer()
{
	if (BufferInfo.BufferInput != EAttackInputType::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ComboComponent] 저장되어 있는 버퍼 입력 사용"));
		
		EAttackInputType SavedInput = BufferInfo.BufferInput;
		
		ResetBuffer();
		
		RegisterInput(SavedInput);
	}
}

void UKOComboComponent::ResetBuffer()
{
	BufferInfo.BufferInput = EAttackInputType::None;
	BufferInfo.RemainingTime = 0.0f;
	CurrentComboRowName = NAME_None;
}

bool UKOComboComponent::TryExcuteAttack(EAttackInputType InputType)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !ComboDataTable)
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
	
	FGameplayTag ExcuteTag = KOGameplayTags::Event_Attack_Excute;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, ExcuteTag, Payload);\
	
	CurrentComboRowName = NextRowName;
	return true;
}

