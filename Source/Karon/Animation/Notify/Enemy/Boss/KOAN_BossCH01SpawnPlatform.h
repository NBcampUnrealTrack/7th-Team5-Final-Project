#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "KOAN_BossCH01SpawnPlatform.generated.h"

class AKOBossCH01Platform;

UCLASS()
class KARON_API UKOAN_BossCH01SpawnPlatform : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;
 
protected:
	// 스폰할 발판 클래스
	UPROPERTY(EditAnywhere, Category = "Platform")
	TSubclassOf<AKOBossCH01Platform> PlatformClass;
 
	// 생성 갯수
	UPROPERTY(EditAnywhere, Category = "Platform | Spawn",
		meta = (ClampMin = "1"))
	int32 SpawnCount = 3;
 
	// 보스와 발판 사이 최소 거리
	UPROPERTY(EditAnywhere, Category = "Platform | Spawn",
		meta = (ClampMin = "0"))
	float MinDistanceFromBoss = 200.f;
 
	// 보스와 발판 사이 최대 거리
	UPROPERTY(EditAnywhere, Category = "Platform | Spawn",
		meta = (ClampMin = "0"))
	float MaxDistanceFromBoss = 600.f;
 
	// 발판 간 최소 거리
	UPROPERTY(EditAnywhere, Category = "Platform | Spawn",
		meta = (ClampMin = "0"))
	float MinDistanceBetweenPlatforms = 150.f;
 
	// 발판 스폰 높이
	UPROPERTY(EditAnywhere, Category = "Platform | Spawn")
	float SpawnHeight = 800.f;
 
	// 위치 선정 최대 시도 횟수
	UPROPERTY(EditAnywhere, Category = "Platform | Spawn", meta = (ClampMin = "1"))
	int32 MaxPlacementAttempts = 10;
 
private:
	bool FindSpawnLocation(
		const FVector& BossLocation,
		const TArray<FVector>& SpawnedLocations,
		FVector& OutLocation
	) const;
};
