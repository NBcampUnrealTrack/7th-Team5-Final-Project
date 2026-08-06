#include "KO_ABonfire.h"
#include "Subsystem/KOTeleportSubsystem.h"
#include "UI/KOUISubsystem.h"
#include "AbilitySystem/Tag/UI/KOGameplayTags_UI.h"

#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "Subsystem/KOQuestGuideSubsystem.h"
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
	InteractionBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	
	TeleportTargetComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TeleportTargetComponent"));
	TeleportTargetComponent->SetupAttachment(RootComponent);
	
	BonfireID = NAME_None;
}

void AKO_ABonfire::BeginPlay()
{
	Super::BeginPlay();
	
	if (BonfireID == NAME_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("Bonfire ID 설정 안됨."));
	}
	
	if (UKOTeleportSubsystem* TeleportSubsystem = UKOTeleportSubsystem::Get(this))
	{
		TeleportSubsystem->RegisterBonfire(this);
	}
	
	UpdateBonfireVisuals();
}

bool AKO_ABonfire::CanInteract(AActor* Interactor) const
{
	return bHasItem;
}

void AKO_ABonfire::OnInteract(AActor* Interactor)
{
	if (!Interactor) return;
	
	UKOTeleportSubsystem* TeleportSubsystem = UKOTeleportSubsystem::Get(this);

	if (!TeleportSubsystem)
	{
		return;
	}

	if (!TeleportSubsystem->IsActivated(BonfireID))
	{
		TeleportSubsystem->ActivateBonfire(BonfireID);
		
		UpdateBonfireVisuals();
		
		// 퀘스트
		if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
		{
			QuestGuide->NotifyBonfireActivated(BonfireID);
		}
		
		if (UKOSaveSubsystem* SaveSubsystem = UKOSaveSubsystem::Get(this))
		{
			SaveSubsystem->SaveCurrentGame();
		}
	}
	else
	{
		if (UKOUISubsystem* UISubsystem = UKOUISubsystem::Get(this))
		{
			UISubsystem->OpenWidget(GetWorld(), KOGameplayTags::UI_Widget_TeleportPopup);
			TeleportSubsystem->SetRequestActorName(BonfireID);
		}
	}
}

FText AKO_ABonfire::GetInteractionPrompt() const
{
	if (IsActivated())
	{
		return NSLOCTEXT(
			 "KOTeleportGuide",
			 "Teleport",
			 "비프로스트"
		  );
	}
	else
	{
		return NSLOCTEXT(
			 "KOTeleportGuide",
			 "Activate",
			 "활성화"
		  );
	}
}

bool AKO_ABonfire::IsActivated() const
{
	if (const UKOTeleportSubsystem* TeleportSubsystem = UKOTeleportSubsystem::Get(this))
	{
		return TeleportSubsystem->IsActivated(BonfireID);
	}

	return false;
}

void AKO_ABonfire::UpdateBonfireVisuals()
{
	if (!BonfireMesh) return;
	
	if (IsActivated())
	{
		if (ActiveOverlayMaterial)
		{
			BonfireMesh->SetOverlayMaterial(ActiveOverlayMaterial);
		}
	}
	else
	{
		if (InactiveOverlayMaterial)
		{
			BonfireMesh->SetOverlayMaterial(InactiveOverlayMaterial);
		}
	}
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
	
	// 이동 퀘스트
	if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
	{
		QuestGuide->NotifyBonfireTeleported(TargetID);
	}
}
