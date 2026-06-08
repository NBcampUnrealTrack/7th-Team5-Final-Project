#include "KOAbilitySystemComponent.h"
#include "Karon/Data/Character/KOGrantSet.h"
#include "Utility/Log/KOLogManager.h"

void UKOAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (!AbilitySpec.Ability) continue; 
			
			if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag) || 
				AbilitySpec.Ability->AbilityTags.HasTagExact(InputTag))
			{
				KO_LOG(Input, Log, TEXT("[%s Ability]: Pressed"), *AbilitySpec.Ability->GetName());
				
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
			if (AbilitySpec.Ability && 
				(AbilitySpec.Ability->AbilityTags.HasTagExact(InputTag) || 
					AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)))
			{
				KO_LOG(Input, Log, TEXT("[%s Ability]: Released"), *AbilitySpec.Ability->GetName());
				
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
		}
	}
}

void UKOAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	// 1. Held 재활성화 (조건 회복 시) - 가장 먼저
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (Spec->Ability && !Spec->IsActive())
				TryActivateAbility(SpecHandle);
		}
	}
	
	// 2. Pressed 처리
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (Spec->Ability)
			{
				Spec->InputPressed = true;
				if (Spec->IsActive())
					AbilitySpecInputPressed(*Spec);
				else
					TryActivateAbility(SpecHandle);
			}
		}
	}
	
	// 3. Released 처리
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

	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UKOAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

void UKOAbilitySystemComponent::GiveGrantSet()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	if (!GrantSet) return;
	
	GrantSet->GiveToAsc(this, GrantSetHandle);
}

void UKOAbilitySystemComponent::ClearGrantSet()
{
	GrantSetHandle.RemoveFromASC(this);
}
