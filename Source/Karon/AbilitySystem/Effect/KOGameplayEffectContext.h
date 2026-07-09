#pragma once
#include "Data/KO_HitData.h"
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
		
	TWeakObjectPtr<UKO_HitData> CustomHitData;
	
	void SetHitData(const UKO_HitData* InHitData) 
	{ 
		CustomHitData = const_cast<UKO_HitData*>(InHitData); 
	}
    
	const UKO_HitData* GetHitData() const 
	{ 
		return CustomHitData.Get(); 
	}
	
protected:
	UPROPERTY()
	bool bIsCriticalHit = false;
};

