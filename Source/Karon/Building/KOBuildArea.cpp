#include "KOBuildArea.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#include "Component/Build/KOBuildUIComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogKOBuildArea, Log, All);

AKOBuildArea::AKOBuildArea()
{
	PrimaryActorTick.bCanEverTick = false;

	BuildAreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BuildAreaBox"));
	SetRootComponent(BuildAreaBox);

	BuildAreaBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BuildAreaBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	BuildAreaBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	BuildAreaBox->SetGenerateOverlapEvents(true);
}

void AKOBuildArea::BeginPlay()
{
	Super::BeginPlay();

	BuildAreaBox->OnComponentEndOverlap.AddDynamic(this, &AKOBuildArea::OnBuildAreaEndOverlap);
}

void AKOBuildArea::OnBuildAreaEndOverlap(
	UPrimitiveComponent* /*OverlappedComponent*/,
	AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/,
	int32 /*OtherBodyIndex*/
)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC)
	{
		return;
	}

	UKOBuildUIComponent* BuildUIComponent = PC->FindComponentByClass<UKOBuildUIComponent>();
	if (!BuildUIComponent)
	{
		return;
	}

	// 건설 모드가 꺼져 있으면 아무것도 하지 않음
	if (!BuildUIComponent->IsBuildMenuOpen())
	{
		return;
	}

	BuildUIComponent->CloseBuildMenu();

	UE_LOG(
		LogKOBuildArea,
		Log,
		TEXT("[BuildArea] 플레이어가 건설 지역을 이탈하여 건설 모드를 종료했습니다.")
	);
}