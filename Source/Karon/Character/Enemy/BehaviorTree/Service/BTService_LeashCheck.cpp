// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_LeashCheck.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTService_LeashCheck::UBTService_LeashCheck()
{
	NodeName = TEXT("Find Random Location");

	InitialLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_LeashCheck, InitialLocationKey));
	MaxDistanceKey.AddFloatFilter(this,GET_MEMBER_NAME_CHECKED(UBTService_LeashCheck, MaxDistanceKey));
	bIsReturnKey.AddBoolFilter(this,GET_MEMBER_NAME_CHECKED(UBTService_LeashCheck, bIsReturnKey));
	SelfActorKey.AddObjectFilter(this,GET_MEMBER_NAME_CHECKED(UBTService_LeashCheck, SelfActorKey),AActor::StaticClass());

}

void UBTService_LeashCheck::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaTime)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaTime);
	
	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	FVector InitialLocation = BBComp->GetValueAsVector(InitialLocationKey.SelectedKeyName);
	float MaxDistance = BBComp->GetValueAsFloat(MaxDistanceKey.SelectedKeyName);
	UObject* EnemyObject = BBComp->GetValueAsObject(SelfActorKey.SelectedKeyName);
	FVector CurrentLocation=FVector::ZeroVector;
	if (EnemyObject)
	{
		//Actor인건 자명하니, 비용을 줄이기 위해 static_cast
		CurrentLocation=static_cast<AActor*>(EnemyObject)->GetActorLocation();
	}
	
	 if (FVector::DistSquared(CurrentLocation, InitialLocation) > FMath::Square(MaxDistance) )
	{
	 	
		BBComp->SetValueAsBool(bIsReturnKey.SelectedKeyName, true);
	}
	
}
