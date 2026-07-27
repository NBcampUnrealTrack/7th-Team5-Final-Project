#include "KO_ABonfire.h"

#include "Components/BoxComponent.h"


AKO_ABonfire::AKO_ABonfire()
{
	PrimaryActorTick.bCanEverTick = false;
	
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	RootComponent = RootComp;

	BonfireMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BonfireMesh"));
	BonfireMesh->SetupAttachment(RootComponent);
    
	BonfireMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(RootComponent);
	InteractionBox->SetBoxExtent(FVector(150.f, 150.f, 100.f));
	
	bIsActivated = false;
	BonfireID = NAME_None;
}

void AKO_ABonfire::BeginPlay()
{
	Super::BeginPlay();
	
	if (BonfireID == NAME_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("Bonfire ID 설정 안됨."));
	}
	
	LoadBonfireState();
}

void AKO_ABonfire::Interact()
{
	if (!bIsActivated)
	{
		bIsActivated = true;
		SaveBonfireState();
	}
	else
	{
		// TODO: 화톳불 UI 띄우기
	}
}

void AKO_ABonfire::SaveBonfireState()
{
}

void AKO_ABonfire::LoadBonfireState()
{
}


