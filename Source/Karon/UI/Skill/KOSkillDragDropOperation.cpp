// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Skill/KOSkillDragDropOperation.h"

#include "Components/Image.h"

UKOSkillDragDropOperation* UKOSkillDragDropOperation::Create(
	UObject* Outer,
	FName InSkillName,
	FGameplayTag InSkillTag,
	UTexture2D* InIcon,
	const FVector2D& InDragVisualSize,
	float InDragVisualOpacity
)
{
	if (!Outer || InSkillName.IsNone())
	{
		return nullptr;
	}

	UKOSkillDragDropOperation* Op = NewObject<UKOSkillDragDropOperation>(Outer);
	if (!Op)
	{
		return nullptr;
	}

	Op->SkillName = InSkillName;
	Op->SkillTag  = InSkillTag;
	Op->Icon      = InIcon;

	if (InIcon)
	{
		UImage* DragVisualImage = NewObject<UImage>(Op);
		if (DragVisualImage)
		{
			DragVisualImage->SetBrushFromTexture(InIcon);
			DragVisualImage->SetDesiredSizeOverride(InDragVisualSize);
			DragVisualImage->SetOpacity(InDragVisualOpacity);
			Op->DefaultDragVisual = DragVisualImage;
		}
	}

	Op->Pivot = EDragPivot::CenterCenter;

	return Op;
}