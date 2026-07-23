#include "Animation/Notify/Enemy/Boss/KOAN_BossCH01SpawnPlatform.h"

#include "Character/Enemy/Boss/Chapter01/Gimmick/KOBossCH01Platform.h"

void UKOAN_BossCH01SpawnPlatform::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}
 
	AActor* Owner = MeshComp->GetOwner();
 
	if (!PlatformClass)
	{
		return;
	}
 
	const FVector BossLocation = Owner->GetActorLocation();
	TArray<FVector> SpawnedLocations;
 
	int32 SuccessCount = 0;
 
	for (int32 i = 0; i < SpawnCount; i++)
	{
		FVector SpawnLocation;
 
		// 유효한 위치 탐색
		if (!FindSpawnLocation(BossLocation, SpawnedLocations, SpawnLocation))
		{
			continue;
		}
 
		// 발판 스폰
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Owner;
 
		AKOBossCH01Platform* Platform = Owner->GetWorld()->SpawnActor<AKOBossCH01Platform>(
			PlatformClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			SpawnParams
		);
 
		if (Platform)
		{
			SpawnedLocations.Add(SpawnLocation);
			SuccessCount++;
		}
	}
}
 
bool UKOAN_BossCH01SpawnPlatform::FindSpawnLocation(
	const FVector& BossLocation,
	const TArray<FVector>& SpawnedLocations,
	FVector& OutLocation) const
{
	for (int32 Attempt = 0; Attempt < MaxPlacementAttempts; Attempt++)
	{
		const float RandomAngle = FMath::RandRange(0.f, 360.f);
		const float RandomDistance = FMath::RandRange(
			MinDistanceFromBoss, MaxDistanceFromBoss
		);
 
		FVector Candidate = BossLocation
			+ FVector(
				FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
				FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
				SpawnHeight
			);

		bool bTooClose = false;
		for (const FVector& Existing : SpawnedLocations)
		{
			const float Dist = FVector::Dist2D(Candidate, Existing);
			if (Dist < MinDistanceBetweenPlatforms)
			{
				bTooClose = true;
				break;
			}
		}
 
		if (bTooClose)
		{
			continue;
		}
 
		OutLocation = Candidate;
		return true;
	}
 
	return false;
}
