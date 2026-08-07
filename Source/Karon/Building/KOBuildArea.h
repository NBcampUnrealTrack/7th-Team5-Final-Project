#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOBuildArea.generated.h"

class UBoxComponent;

UCLASS()
class KARON_API AKOBuildArea : public AActor
{
	GENERATED_BODY()

public:
	AKOBuildArea();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBuildAreaEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build|Area")
	TObjectPtr<UBoxComponent> BuildAreaBox;
};