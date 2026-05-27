#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/Boss/KOGA_BossAttackBase.h"
#include "KOGA_BossMeleeAttackBase.generated.h"

UCLASS()
class KARON_API UKOGA_BossMeleeAttackBase : public UKOGA_BossAttackBase
{
	GENERATED_BODY()

public:
	UKOGA_BossMeleeAttackBase();
 
	// NotifyState에서 소켓 위치 가져올 때 사용
	FVector GetAttackSocketLocation() const;
 
	// 공격에 사용할 소켓 이름 (BP에서 공격마다 설정)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack | Socket")
	FName AttackSocketName = FName("hand_r");
};
