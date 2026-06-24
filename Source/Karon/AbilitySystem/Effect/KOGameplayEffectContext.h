#pragma once
#include "GameplayEffectTypes.h"
#include "KOGameplayEffectContext.generated.h"

USTRUCT()
struct KARON_API FKOGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	bool IsCriticalHit() const { return bIsCriticalHit; }
	void SetIsCriticalHit(bool bInIsCritical) { bIsCriticalHit = bInIsCritical; }

	virtual UScriptStruct* GetScriptStruct() const override;
	virtual FKOGameplayEffectContext* Duplicate() const override;
	
protected:
	UPROPERTY()
	bool bIsCriticalHit = false;
};

