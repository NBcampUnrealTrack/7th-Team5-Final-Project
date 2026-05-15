#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "Karon/Data/KOInputConfig.h"
#include "KOInputComponent.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;

UCLASS()
class KARON_API UKOInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
	
	public:
	UKOInputComponent(const FObjectInitializer& ObjectInitializer);
	
	void AddInputMapping(const UKOInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubSystem);
	
	void RemoveInputMappings(
		const UKOInputConfig* InputConfig,
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem
	) const;
	
	void RemoveBinds(TArray<uint32>& BindHandles);
	
public:
	template<class UserClass, typename FuncType>
	void BindNativeAction(
		const UKOInputConfig* InputConfig,
		const FGameplayTag& InputTag,
		ETriggerEvent TriggerEvent, 
		UserClass* Object, 
		FuncType Func, 
		bool bLogIfNotFound
	);

	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(
		const UKOInputConfig* InputConfig,
		UserClass* Object, 
		PressedFuncType PressedFunc, 
		ReleasedFuncType ReleasedFunc, 
		TArray<uint32>& BindHandles
	);
};


template <class UserClass, typename FuncType>
void UKOInputComponent::BindNativeAction(const UKOInputConfig* InputConfig, const FGameplayTag& InputTag,
	ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
	check(InputConfig);
	
	if (const auto* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
	{
		BindAction(IA, TriggerEvent, Object, Func);	
	}
}

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UKOInputComponent::BindAbilityActions(const UKOInputConfig* InputConfig, UserClass* Object,
	PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
{
	check(InputConfig);

	for (const auto& Action : InputConfig->AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag).GetHandle());
			}
			
			if (ReleasedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
			}
		}
	}
};
