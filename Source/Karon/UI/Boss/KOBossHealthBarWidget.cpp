#include "UI/Boss/KOBossHealthBarWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/KOHealthSet.h"
#include "Character/Enemy/Boss/KOBossBase.h"
#include "Character/Enemy/Boss/KOBossDataAsset.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UKOBossHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UKOBossHealthBarWidget::NativeDestruct()
{
	if (BossRef)
	{
		UAbilitySystemComponent* ASC = BossRef->GetAbilitySystemComponent();
		if (ASC)
		{
			UKOHealthSet* HealthSet = const_cast<UKOHealthSet*>(ASC->GetSet<UKOHealthSet>());
			if (HealthSet)
			{
				HealthSet->OnHealthChanged.RemoveDynamic(this, &UKOBossHealthBarWidget::OnHealthChanged);
			}
		}
	}
	
	Super::NativeDestruct();
}

void UKOBossHealthBarWidget::SetBoss(AKOBossBase* InBoss)
{
	if (!InBoss)
	{
		return;
	}
	
	UAbilitySystemComponent* ASC = InBoss->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}
	
	UKOHealthSet* HealthSet = const_cast<UKOHealthSet*>(ASC->GetSet<UKOHealthSet>());
	if (!HealthSet)
	{
		return;
	}
	
	if (BossName && InBoss->GetDataAsset())
	{
		BossName->SetText(FText::FromName(InBoss->GetDataAsset()->BossName));
	}
	
	HealthSet->OnHealthChanged.AddUniqueDynamic(this, &UKOBossHealthBarWidget::OnHealthChanged);
	UpdateHealthBar(HealthSet->GetHealth(), HealthSet->GetMaxHealth());
}

void UKOBossHealthBarWidget::OnHealthChanged(float OldVal, float NewVal)
{
	if (!BossRef)
	{
		return;
	}
 
	UAbilitySystemComponent* ASC = BossRef->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}
 
	const UKOHealthSet* HealthSet = ASC->GetSet<UKOHealthSet>();
	if (!HealthSet)
	{
		return;
	}
 
	UpdateHealthBar(NewVal, HealthSet->GetMaxHealth());
}

void UKOBossHealthBarWidget::UpdateHealthBar(float Current, float Max)
{
	if (!BossHealth || Max <= 0.f)
	{
		return;
	}
 
	BossHealth->SetPercent(Current / Max);
}
