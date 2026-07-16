// Copyright Karon Team 5. All Rights Reserved.

#include "UI/Inventory/KOCharacterStatWidget.h"

#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"

void UKOCharacterStatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeFromPawn(GetOwningPlayerPawn());

	if (APlayerController* OwningPC = GetOwningPlayer())
	{
		OwningPC->OnPossessedPawnChanged.AddDynamic(this, &UKOCharacterStatWidget::HandlePossessedPawnChanged);
	}
}

void UKOCharacterStatWidget::NativeDestruct()
{
	if (APlayerController* OwningPC = GetOwningPlayer())
	{
		OwningPC->OnPossessedPawnChanged.RemoveDynamic(this, &UKOCharacterStatWidget::HandlePossessedPawnChanged);
	}

	UnbindFromAbilitySystem();

	Super::NativeDestruct();
}

void UKOCharacterStatWidget::SetTargetPawn(APawn* InPawn)
{
	InitializeFromPawn(InPawn);
}

void UKOCharacterStatWidget::InitializeFromPawn(APawn* Pawn)
{
	UnbindFromAbilitySystem();

	UAbilitySystemComponent* ASC = Pawn ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn) : nullptr;
	if (!ASC)
	{
		return;
	}

	BindToAbilitySystem(ASC);
}

void UKOCharacterStatWidget::BindToAbilitySystem(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return;
	}

	CachedASC = ASC;

	CachedCombatSet = const_cast<UKOCombatSet*>(ASC->GetSet<UKOCombatSet>());
	if (CachedCombatSet)
	{
		CachedCombatSet->OnAttackPowerChanged.AddUniqueDynamic(this, &UKOCharacterStatWidget::OnAttackPowerChanged);
		CachedCombatSet->OnDefenseChanged.AddUniqueDynamic(this, &UKOCharacterStatWidget::OnDefenseChanged);
	}

	RefreshCombat();
}

void UKOCharacterStatWidget::UnbindFromAbilitySystem()
{
	if (CachedCombatSet)
	{
		CachedCombatSet->OnAttackPowerChanged.RemoveDynamic(this, &UKOCharacterStatWidget::OnAttackPowerChanged);
		CachedCombatSet->OnDefenseChanged.RemoveDynamic(this, &UKOCharacterStatWidget::OnDefenseChanged);
		CachedCombatSet = nullptr;
	}

	CachedASC = nullptr;
}

void UKOCharacterStatWidget::RefreshCombat()
{
	if (!CachedCombatSet)
	{
		return;
	}

	if (AttackPowerText)
	{
		AttackPowerText->SetText(FText::AsNumber(CachedCombatSet->GetAttackPower()));
	}

	if (DefenseText)
	{
		DefenseText->SetText(FText::AsNumber(CachedCombatSet->GetDefense()));
	}
}

void UKOCharacterStatWidget::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	InitializeFromPawn(NewPawn);
}

void UKOCharacterStatWidget::OnAttackPowerChanged(float OldValue, float NewValue)
{
	RefreshCombat();
}

void UKOCharacterStatWidget::OnDefenseChanged(float OldValue, float NewValue)
{
	RefreshCombat();
}