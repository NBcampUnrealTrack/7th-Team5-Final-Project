// Copyright Karon Team 5. All Rights Reserved.

#include "KOInGameHUD.h"
#include "Character/Hero/KOHeroCharacter.h"
#include "AbilitySystem/Attribute/KOHealthSet.h"
#include "AbilitySystem/Attribute/KOStaminaSet.h"
#include "AbilitySystem/Tag/Data/KOGameplayTags_Data.h"
#include "Utility/Messaging/KOMessageTypes.h"
#include "StructUtils/InstancedStruct.h"

#include "Components/ProgressBar.h"

void UKOInGameHUD::SetBuildKeyGuideMode(bool bBuildMode)
{
	UE_LOG(LogTemp, Warning, TEXT("[HUD] SetBuildKeyGuideMode bBuildMode=%d"), bBuildMode ? 1 : 0);
	if (NormalKeyGuide)
	{
		NormalKeyGuide->SetVisibility(
			bBuildMode
				? ESlateVisibility::Collapsed
				: ESlateVisibility::HitTestInvisible
		);
	}

	if (BuildKeyGuide)
	{
		BuildKeyGuide->SetVisibility(
			bBuildMode
				? ESlateVisibility::HitTestInvisible
				: ESlateVisibility::Collapsed
		);
	}
}

void UKOInGameHUD::BP_OnSkillQuickSlotChanged_Implementation(ESkillQuickSlotKey SlotKey, FName SkillName)
{
	// Blueprint에서 오버라이드해 HUD 내 스킬 슬롯 UI를 갱신한다.
}

void UKOInGameHUD::HandleSkillQuickSlotChangedMessage(FGameplayTag Channel, const FInstancedStruct& Payload)
{
	const FKOSkillQuickSlotChangedMessage* Msg = Payload.GetPtr<FKOSkillQuickSlotChangedMessage>();
	if (!Msg)
	{
		return;
	}

	BP_OnSkillQuickSlotChanged(Msg->SlotKey, Msg->SkillName);
}

void UKOInGameHUD::NativeConstruct()
{
	Super::NativeConstruct();

	SkillQuickSlotChangedCallback.BindDynamic(
		this, &UKOInGameHUD::HandleSkillQuickSlotChangedMessage);
	SkillQuickSlotChangedHandle = Subscribe(
		KOGameplayTags::Data_Message_Skill_QuickSlotChanged,
		SkillQuickSlotChangedCallback);

	AKOHeroCharacter* HeroCharacter = Cast<AKOHeroCharacter>(GetOwningPlayerPawn());
	if (HeroCharacter == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("InGameHUD: 히어로 캐릭터를 찾지 못했습니다."));
		return;
	}

	CachedHealthSet = HeroCharacter->GetHealthSet();
	if (CachedHealthSet)
	{
		CachedCurrentHealth = CachedHealthSet->GetHealth();
		CachedMaxHealth = CachedHealthSet->GetMaxHealth();
		RefreshHealthBar();

		CachedHealthSet->OnHealthChanged.AddDynamic(this, &UKOInGameHUD::OnCurrentHealthChanged);
		CachedHealthSet->OnMaxHealthChanged.AddDynamic(this, &UKOInGameHUD::OnMaxHealthChanged);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("InGameHUD: 캐릭터의 HealthSet을 가져오는 데 실패했습니다."));
	}
	
	CachedStaminaSet = HeroCharacter->GetStaminaSet();
	if (CachedStaminaSet)
	{
		CachedCurrentStamina = CachedStaminaSet->GetStamina();
		CachedMaxStamina = CachedStaminaSet->GetMaxStamina();
		RefreshStaminaBar();

		CachedStaminaSet->OnStaminaChanged.AddDynamic(this, &UKOInGameHUD::OnCurrentStaminaChanged);
		CachedStaminaSet->OnMaxStaminaChanged.AddDynamic(this, &UKOInGameHUD::OnMaxStaminaChanged);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("InGameHUD: 캐릭터의 StaminaSet을 가져오는 데 실패했습니다."));
	}
}

void UKOInGameHUD::NativeDestruct()
{
	Unsubscribe(SkillQuickSlotChangedHandle);
	SkillQuickSlotChangedCallback.Clear();

	if (IsValid(CachedHealthSet))
	{
		CachedHealthSet->OnHealthChanged.RemoveDynamic(this, &UKOInGameHUD::OnCurrentHealthChanged);
		CachedHealthSet->OnMaxHealthChanged.RemoveDynamic(this, &UKOInGameHUD::OnMaxHealthChanged);
	}
	
	if (IsValid(CachedStaminaSet))
	{
		CachedStaminaSet->OnStaminaChanged.RemoveDynamic(this, &UKOInGameHUD::OnCurrentStaminaChanged);
		CachedStaminaSet->OnMaxStaminaChanged.RemoveDynamic(this, &UKOInGameHUD::OnMaxStaminaChanged);
	}
	
	CachedHealthSet  = nullptr;
	CachedStaminaSet = nullptr;
	
	Super::NativeDestruct();
}

void UKOInGameHUD::OnCurrentHealthChanged(float OldValue, float NewValue)
{
	CachedCurrentHealth = NewValue;
	RefreshHealthBar();
}

void UKOInGameHUD::OnMaxHealthChanged(float OldValue, float NewValue)
{
	CachedMaxHealth = NewValue;
	RefreshHealthBar();
}

void UKOInGameHUD::RefreshHealthBar()
{
	if (HealthBar && CachedMaxHealth > 0.f)
	{
		HealthBar->SetPercent(CachedCurrentHealth / CachedMaxHealth);
	}
}


void UKOInGameHUD::OnCurrentStaminaChanged(float OldValue, float NewValue)
{
	CachedCurrentStamina = NewValue;
	RefreshStaminaBar();
}

void UKOInGameHUD::OnMaxStaminaChanged(float OldValue, float NewValue)
{
	CachedMaxStamina = NewValue;
	RefreshStaminaBar();
}

void UKOInGameHUD::RefreshStaminaBar()
{
	if (StaminaBar && CachedMaxStamina >0.f)
	{
		StaminaBar->SetPercent(CachedCurrentStamina / CachedMaxStamina);
	}
}
