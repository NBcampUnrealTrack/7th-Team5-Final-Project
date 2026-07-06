// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Type/KOEnemyType.h"
#include "GameFramework/Actor.h"
#include "Utility/Interface/KOInteractableInterface.h"
#include "KOItemDropActor.generated.h"

UCLASS()
class KARON_API AKOItemDropActor : public AActor,public IKOInteractableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AKOItemDropActor();
	
	virtual bool CanInteract(AActor* Interactor) const override;
	
	virtual void OnInteract(AActor* Interactor) override;

	virtual FText GetInteractionPrompt() const override;
	
	bool CheckCanGetItem(float DropPercent);
	
	FName GetDropSaveId() const { return DropSaveId; }
	bool HasItemForSave() const { return bHasItem; }

	void ApplyCollectedFromSave(); // 숨기기
	void ApplyAvailableFromSave(); // 보이기
	
protected:
	bool bHasItem=true;
	
	UPROPERTY(EditAnywhere, Category="ItemDrop")
	TArray<FEnemyDropItemInfo> DropItems;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="KO|Save")
	FName DropSaveId = NAME_None;
	
private:
	void MarkCollectedForSave();
};
