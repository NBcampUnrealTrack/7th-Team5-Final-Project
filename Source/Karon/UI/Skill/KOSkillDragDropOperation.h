// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "GameplayTagContainer.h"
#include "KOSkillDragDropOperation.generated.h"

class UTexture2D;
class UKOSkillQuickSlotEntryWidget;

UCLASS()
class KARON_API UKOSkillDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "KO|DragDrop")
	FName SkillName;

	UPROPERTY(BlueprintReadOnly, Category = "KO|DragDrop")
	FGameplayTag SkillTag;

	UPROPERTY(BlueprintReadOnly, Category = "KO|DragDrop")
	TObjectPtr<UTexture2D> Icon = nullptr;

	/** 드래그 출발지가 퀵슬롯 엔트리면 설정됨 — nullptr이면 SkillNode에서 출발 */
	UPROPERTY(BlueprintReadOnly, Category = "KO|DragDrop")
	TWeakObjectPtr<UKOSkillQuickSlotEntryWidget> SourceSlotWidget;

public:
	static UKOSkillDragDropOperation* Create(
		UObject* Outer,
		FName InSkillName,
		FGameplayTag InSkillTag,
		UTexture2D* InIcon,
		const FVector2D& InDragVisualSize,
		float InDragVisualOpacity
	);
};