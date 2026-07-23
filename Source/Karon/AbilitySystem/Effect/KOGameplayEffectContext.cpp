#include "KOGameplayEffectContext.h"

UScriptStruct* FKOGameplayEffectContext::GetScriptStruct() const
{
	return FKOGameplayEffectContext::StaticStruct();
}

FKOGameplayEffectContext* FKOGameplayEffectContext::Duplicate() const
{
	FKOGameplayEffectContext* NewContext = new FKOGameplayEffectContext();
	*NewContext = *this;
	if (GetHitResult())
		NewContext->AddHitResult(*GetHitResult(), true);
	return NewContext;
}
