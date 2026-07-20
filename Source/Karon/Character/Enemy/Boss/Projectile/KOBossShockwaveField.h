#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/Boss/Projectile/KOBossProjectileBase.h"
#include "KOBossShockwaveField.generated.h"

class UNiagaraSystem;

UCLASS()
class KARON_API AKOBossShockwaveField : public AKOBossProjectileBase
{
	GENERATED_BODY()
	
public:
	AKOBossShockwaveField();
 
	virtual void BeginPlay() override;
 
protected:
	// 점프 회피 가능 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shockwave | Option")
	bool bCanDodgeByJump = true;
 
	// 충격파 반경
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shockwave | Option")
	float ShockwaveRadius = 300.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shockwave | VFX")
	TObjectPtr<UNiagaraSystem> ShockwaveVFX;
};
