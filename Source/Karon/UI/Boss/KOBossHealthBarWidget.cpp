#include "UI/Boss/KOBossHealthBarWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/KOGroggySet.h"
#include "AbilitySystem/Attribute/KOHealthSet.h"
#include "Animation/WidgetAnimation.h"
#include "Character/Enemy/Boss/KOBossBase.h"
#include "Character/Enemy/Boss/KOBossDataAsset.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UKOBossHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	SetVisibility(ESlateVisibility::Hidden);
	
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(this, AKOBossBase::StaticClass(), Actors);
	

	for (AActor* Actor : Actors)
	{
		AKOBossBase* Boss = Cast<AKOBossBase>(Actor);
		if (!Boss)
		{
			continue;
		}

		Boss->OnBossDetectedPlayer.AddUObject(this, &UKOBossHealthBarWidget::OnBossDetected);

		Boss->OnBossDied.AddUObject(this, &UKOBossHealthBarWidget::OnBossDiedCallback);
	}
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
		
		UKOGroggySet* GroggySet = const_cast<UKOGroggySet*>(ASC->GetSet<UKOGroggySet>());
		if (GroggySet)
		{
			GroggySet->OnGroggyHealthChanged.RemoveDynamic(this, &UKOBossHealthBarWidget::OnGroggyChanged);
		}
	}
	
	Super::NativeDestruct();
}

void UKOBossHealthBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (BossRef && GetVisibility() == ESlateVisibility::Visible)
	{
		APlayerController* PC = GetOwningPlayer();
		APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;

		if (PlayerPawn)
		{
			const float Distance = FVector::Dist(
				PlayerPawn->GetActorLocation(),
				BossRef->GetActorLocation()
			);

			const bool bShouldBeVisible = Distance <= MaxVisibleDistance;
			const float CurrentOpacity = GetRenderOpacity();

			if (bShouldBeVisible && CurrentOpacity < 0.5f && !IsAnimationPlaying(FadeIn))
			{
				if (FadeIn) PlayAnimation(FadeIn);
			}
			else if (!bShouldBeVisible && CurrentOpacity > 0.5f && !IsAnimationPlaying(FadeOut))
			{
				if (FadeOut) PlayAnimation(FadeOut);
			}
		}
	}

	if (!BossHealthYellow || MaxHP <= 0.f)
	{
		return;
	}
	if (YellowHP <= CurrentHP)
	{
		return;
	}

	if (!bYellowDecreasing)
	{
		TimeSinceLastHit += InDeltaTime;
		if (TimeSinceLastHit >= YellowDelayTime)
		{
			bYellowDecreasing = true;
		}
	}
	else
	{
		YellowHP = FMath::FInterpConstantTo(YellowHP, CurrentHP, InDeltaTime, MaxHP * YellowDecreaseSpeed);

		BossHealthYellow->SetPercent(FMath::Clamp(YellowHP / MaxHP, 0.f, 1.f));

		if (FMath::IsNearlyEqual(YellowHP, CurrentHP, 0.1f))
		{
			YellowHP = CurrentHP;
			bYellowDecreasing = false;
		}
	}
}

void UKOBossHealthBarWidget::OnBossDetected(AKOBossBase* DetectedBoss)
{
	if (!DetectedBoss)
	{
		return;
	}
	
	SetBoss(DetectedBoss);
	
	SetVisibility(ESlateVisibility::Visible);
	
	if (FadeIn)
	{
		PlayAnimation(FadeIn);
	}
}

void UKOBossHealthBarWidget::OnBossDiedCallback()
{
	BossRef = nullptr;
	
	if (FadeOut)
	{
		PlayAnimation(FadeOut);
		GetWorld()->GetTimerManager().SetTimer(
			FadeOutTimerHandle,
			this,
			&UKOBossHealthBarWidget::OnFadeOutFinished,
			FadeOut->GetEndTime(),
			false
		);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
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
	
	BossRef = InBoss;
	MaxHP = HealthSet->GetMaxHealth();
	CurrentHP = HealthSet->GetHealth();
	YellowHP = CurrentHP;
	TimeSinceLastHit = 0.f;
	bYellowDecreasing = false;
	
	if (BossName && InBoss->GetDataAsset())
	{
		BossName->SetText(FText::FromName(InBoss->GetDataAsset()->BossName));
	}
	
	HealthSet->OnHealthChanged.AddUniqueDynamic(this, &UKOBossHealthBarWidget::OnHealthChanged);
	if (BossHealth)
	{
		BossHealth->SetPercent(FMath::Clamp(CurrentHP / MaxHP, 0.f, 1.f));
	}

	if (BossHealthYellow)
	{
		BossHealthYellow->SetPercent(FMath::Clamp(CurrentHP / MaxHP, 0.f, 1.f));
	}
	
	UKOGroggySet* GroggySet = const_cast<UKOGroggySet*>(ASC->GetSet<UKOGroggySet>());
	if (GroggySet)
	{
		MaxGroggy = GroggySet->GetMaxGroggyHealth();
		CurrentGroggy = GroggySet->GetGroggyHealth();

		GroggySet->OnGroggyHealthChanged.AddUniqueDynamic(
			this, &UKOBossHealthBarWidget::OnGroggyChanged);

		if (BossGroggy && MaxGroggy > 0.f)
		{
			BossGroggy->SetPercent(FMath::Clamp(CurrentGroggy / MaxGroggy, 0.f, 1.f));
		}
	}
}

void UKOBossHealthBarWidget::OnHealthChanged(float OldVal, float NewVal)
{
	if (!BossRef || MaxHP <= 0.f)
	{
		return;
	}

	CurrentHP = NewVal;
	if (BossHealth)
	{
		BossHealth->SetPercent(FMath::Clamp(CurrentHP / MaxHP, 0.f, 1.f));
	}
 
	TimeSinceLastHit = 0.f;
	bYellowDecreasing = false;
}

void UKOBossHealthBarWidget::OnGroggyChanged(float OldVal, float NewVal)
{
	if (!BossGroggy || MaxGroggy <= 0.f)
	{
		return;
	}

	CurrentGroggy = NewVal;
	BossGroggy->SetPercent(FMath::Clamp(CurrentGroggy / MaxGroggy, 0.f, 1.f));
}

void UKOBossHealthBarWidget::UpdateHealthBar(float Current, float Max)
{
	if (Max <= 0.f)
	{
		return;
	}
	
	if (BossHealth)
	{
		BossHealth->SetPercent(FMath::Clamp(Current / Max, 0.f, 1.f));
	}
}

void UKOBossHealthBarWidget::OnFadeOutFinished()
{
	SetVisibility(ESlateVisibility::Hidden);
}
