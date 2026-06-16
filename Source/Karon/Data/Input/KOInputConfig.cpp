#include "KOInputConfig.h"

UKOInputConfig::UKOInputConfig(const FObjectInitializer& ObjectInitializer)
{
}

const UInputAction* UKOInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FKOInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}
	
	if (bLogNotFound)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."),
			*InputTag.ToString(), 
			*GetNameSafe(this)
		);
	}
	
	return nullptr; 
}

const UInputAction* UKOInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FKOInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(
			LogTemp, 
			Error,
			TEXT("Can't find AbilityInputAction for InputTag [%s] on InputConfig [%s]."),
			*InputTag.ToString(),
			*GetNameSafe(this)
		);
	}

	return nullptr;
}
