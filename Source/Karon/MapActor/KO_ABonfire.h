#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utility/Interface/KOInteractableInterface.h"
#include "KO_ABonfire.generated.h"

class UBoxComponent;
class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class KARON_API AKO_ABonfire : public AActor ,public IKOInteractableInterface
{
	GENERATED_BODY()

public:
	AKO_ABonfire();

protected:
	virtual void BeginPlay() override;

public:
	virtual bool CanInteract(AActor* Interactor) const override;
	
	virtual void OnInteract(AActor* Interactor) override;

	virtual FText GetInteractionPrompt() const override;
	
	const FName& GetBonfireID() const;
	const FText& GetDisplayName() const;
	bool IsActivated() const;
	
	UFUNCTION(BlueprintCallable, Category = "Bonfire|Teleport")
	void TeleportToTargetBonfire(FName TargetID, ACharacter* PlayerCharacter);
	
	void RestoreFromSave(bool bActivated);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RootComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BonfireMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> InteractionBox;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> TeleportTargetComponent;
	
	bool bHasItem = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bonfire Setting")
	FName BonfireID;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bonfire Setting")
	FText DisplayName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bonfire Setting")
	bool bIsActivated;
};
