#include "KOComboComponent.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystem/KOAbilitySystemComponent.h"
#include "AbilitySystem/Tag/Data/KOGameplayTags_Data.h"
#include "AbilitySystem/Tag/Input/KOGameplayTags_Input.h"


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
}

bool UKOComboComponent::TryExcuteAttack(EAttackInputType InputType)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}
	
	UAbilitySystemComponent* ASC = 
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerActor);
	if (!ASC)
	{
		return false;
	}
	
	bool bActivationSuccess = false;
	
	if (InputType == EAttackInputType::Light)
	{
		bActivationSuccess = 
			ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(KOGameplayTags::Input_Ability_Attack_Light));
	}
	else if (InputType == EAttackInputType::Heavy)
	{
		bActivationSuccess = 
			ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(KOGameplayTags::Input_Ability_Attack_Heavy));
	}
		
	return bActivationSuccess;
}

