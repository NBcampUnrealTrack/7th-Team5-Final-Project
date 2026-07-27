#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KO_ABonfire.generated.h"

class UBoxComponent;
class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class KARON_API AKO_ABonfire : public AActor
{
	GENERATED_BODY()

public:
	AKO_ABonfire();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Bonfire")
	void Interact();
	
protected:
	void SaveBonfireState();
	
	void LoadBonfireState();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RootComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BonfireMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> InteractionBox;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> TeleportTargetComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bonfire Setting")
	FName BonfireID;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bonfire Setting")
	bool bIsActivated;
};
