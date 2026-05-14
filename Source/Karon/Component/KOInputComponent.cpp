#include "KOInputComponent.h"
#include "EnhancedInputSubsystems.h"

UKOInputComponent::UKOInputComponent(const FObjectInitializer& ObjectInitializer)
{
}

void UKOInputComponent::AddInputMapping(
	const UKOInputConfig* InputConfig,
	UEnhancedInputLocalPlayerSubsystem* InputSubSystem)
{
	check(InputConfig);
	check(InputSubSystem);
}

void UKOInputComponent::RemoveInputMappings(
	const UKOInputConfig* InputConfig,
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);
}

void UKOInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	
	BindHandles.Reset();
}
