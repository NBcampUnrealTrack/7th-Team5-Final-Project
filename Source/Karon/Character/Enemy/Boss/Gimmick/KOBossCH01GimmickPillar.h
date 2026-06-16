#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOBossCH01GimmickPillar.generated.h"

UCLASS()
class KARON_API AKOBossCH01GimmickPillar : public AActor
{
	GENERATED_BODY()
	
public:
	AKOBossCH01GimmickPillar();
 
protected:
	UPROPERTY(VisibleAnywhere, Category = "Pillar | Component")
	TObjectPtr<UStaticMeshComponent> PillarMesh;
 
	virtual void BeginPlay() override;
};
