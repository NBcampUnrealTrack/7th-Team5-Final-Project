#include "KO_ABonfire.h"
#include "Subsystem/KOTeleportSubsystem.h"
#include "UI/KOUISubsystem.h"
#include "AbilitySystem/Tag/UI/KOGameplayTags_UI.h"

#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "Subsystem/KOSaveSubsystem.h"


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
	
	TeleportTargetComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TeleportTargetComponent"));
	TeleportTargetComponent->SetupAttachment(RootComponent);
	
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
	
	UWorld* World = GetWorld();
	
	if (World == nullptr)
	{
		return;
	}
	
	ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController();
	if (UKOTeleportSubsystem* TeleportSubsystem = LocalPlayer->GetSubsystem<UKOTeleportSubsystem>())
	{
		TeleportSubsystem->RegisterBonfire(this);
	}
}

bool AKO_ABonfire::CanInteract(AActor* Interactor) const
{
	return bHasItem;
}

void AKO_ABonfire::OnInteract(AActor* Interactor)
{
	if (!Interactor) return;
	
	if (!bIsActivated)
	{
		bIsActivated = true;

		UKOSaveSubsystem* SaveSubsystem = UKOSaveSubsystem::Get(this);
		if (!SaveSubsystem || !SaveSubsystem->SaveCurrentGame())
		{
			bIsActivated = false;
		}
	}
	else
	{
		if (UKOUISubsystem* UISubsystem = UKOUISubsystem::Get(this))
		{
			UISubsystem->OpenWidget(GetWorld(), KOGameplayTags::UI_Widget_TeleportPopup);
		}
	}
}

FText AKO_ABonfire::GetInteractionPrompt() const
{
	if (bIsActivated)
	{
		return FText::FromString(TEXT("상호작용"));	
	}
	else
	{
		return FText::FromString(TEXT("활성화"));
	}
}

bool AKO_ABonfire::IsActivated() const
{
	return bIsActivated;
}

void AKO_ABonfire::RestoreFromSave(bool bActivated)
{
	bIsActivated = bActivated;
}

void AKO_ABonfire::TeleportToTargetBonfire(FName TargetID, ACharacter* PlayerCharacter)
{
	if (!PlayerCharacter)
	{
		return;
	}
	
	// TODO: 캐싱한 화톳불의 좌표값 넣기
	// FVector TargetLocation = Bonfire->TeleportTargetComponent->GetComponentLocation();
	// FRotator TargetRotation = Bonfire->TeleportTargetComponent->GetComponentRotation();
	// PlayerCharacter->TeleportTo(TargetLocation, TargetRotation);
}
