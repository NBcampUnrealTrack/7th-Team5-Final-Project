// Fill out your copyright notice in the Description page of Project Settings.


#include "KOWorldOverlapActor.h"

#include "Character/Hero/KOHeroCharacter.h"
#include "Components/BoxComponent.h"
#include "Utility/Messaging/KOMessageTypes.h"

AKOWorldOverlapActor::AKOWorldOverlapActor()
{
	OverlapBox=CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox"));
}

void AKOWorldOverlapActor::BeginPlay()
{
	Super::BeginPlay();
	OverlapBox->OnComponentBeginOverlap.AddDynamic(this,&ThisClass::OnBeginOverlap);
}

void AKOWorldOverlapActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AKOHeroCharacter* Player=Cast<AKOHeroCharacter>(OtherActor))
	{
		FKOWorldNoticeMessage NoticeMessage;
		NoticeMessage.WorldName=WorldName;
	
		UGMRouterSubsystem::BroadcastMessage(GetWorld(),
			KOGameplayTags::Event_WorldNotice,
			FInstancedStruct::Make(NoticeMessage));
	}
}


