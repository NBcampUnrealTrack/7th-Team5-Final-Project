#include "KOAbilitySystemComponent.h"
#include "Karon/Data/KOAbilitySet.h"


void UKOAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.Ability->AbilityTags.HasTagExact(InputTag)))
			{
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		} 
	}
}

void UKOAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && AbilitySpec.Ability->AbilityTags.HasTagExact(InputTag))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
		}
	}
}

void UKOAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	
	// 1. 입력 태그와 일치하는 어빌리티 핸들을 찾습니다.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* Spec  = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (Spec->Ability)
			{
				Spec->InputPressed = true;
				AbilitiesToActivate.AddUnique(SpecHandle); 
			}
		}
	}
	
	// 2. 찾은 어빌리티를 실제로 실행(Activate)합니다.
	for (const FGameplayAbilitySpecHandle& SpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(SpecHandle);
	}
	
	// 3. Released 어빌리티 처리
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (Spec->Ability)
			{
				Spec->InputPressed = false;
				AbilitySpecInputReleased(*Spec); 
			}
		}
	}
	
	// 4. 프레임이 끝날 때 Pressed와 Released 배열은 비워줌 Held는 유지
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UKOAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

void UKOAbilitySystemComponent::GiveDefaultAbilities()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	if (!DefaultAbilitySet) return;
	
	DefaultAbilitySet->GiveAbilities(this);
}
