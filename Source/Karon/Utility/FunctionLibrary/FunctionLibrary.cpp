#include "FunctionLibrary.h"

#include "AbilitySystemComponent.h"
#include "GameplayTagAssetInterface.h"
#include "Character/KOCharacterBase.h"
#include "Kismet/KismetSystemLibrary.h"

void FunctionLibrary::FindActorsWithGameplayTagInRange(
	const UWorld* World, 
	const FVector& ScanOrigin, 
	float Radius,
	const FGameplayTag& TargetTag, 
	const TArray<AActor*>& ActorsToIgnore, 
	TArray<TWeakObjectPtr<AActor>>& OutDetectedActors
	)
{
	if (!World)
	{
		return;
	}
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	TArray<FHitResult> OutHits;

	bool bHasHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
		World, ScanOrigin, ScanOrigin, Radius, ObjectTypes, false, ActorsToIgnore, EDrawDebugTrace::None, OutHits, true
	);

	if (bHasHit)
	{
		for (const FHitResult& Hit : OutHits)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor) continue;
			if (AKOCharacterBase* Character=Cast<AKOCharacterBase>(HitActor))
			{
				if (UAbilitySystemComponent* ASC=Character->GetAbilitySystemComponent())
				{
					if (ASC->HasMatchingGameplayTag(TargetTag))
					{
						OutDetectedActors.Add(HitActor);

					}
				}
			}
		}
	}
};
