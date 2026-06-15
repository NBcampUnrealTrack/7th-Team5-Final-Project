#include "Character/Enemy/Boss/BehaviorTree/KOBTService_FindTaggedLocation.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UKOBTService_FindTaggedLocation::UKOBTService_FindTaggedLocation()
{
	NodeName = TEXT("Find Tagged Location");
	bNotifyTick = true;
	Interval = 0.5f;
 
	LocationKey.AddVectorFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UKOBTService_FindTaggedLocation, LocationKey)
	);
}

void UKOBTService_FindTaggedLocation::TickNode(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
 
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return;
	}
 
	if (bIsLocationFound)
	{
		return;
	}
 
	if (SearchActorTag.IsNone())
	{
		return;
	}
	
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(
		OwnerComp.GetWorld(),
		SearchActorTag,
		FoundActors
	);
 
	if (FoundActors.IsEmpty())
	{
		return;
	}
 
	const FVector Location = FoundActors[0]->GetActorLocation();
	BB->SetValueAsVector(LocationKey.SelectedKeyName, Location);
	bIsLocationFound = true;
}
