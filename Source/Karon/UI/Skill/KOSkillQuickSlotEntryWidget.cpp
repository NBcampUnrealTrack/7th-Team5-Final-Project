// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Skill/KOSkillQuickSlotEntryWidget.h"
#include "UI/Skill/KOSkillDragDropOperation.h"

#include "AbilitySystem/Tag/Data/KOGameplayTags_Data.h"
#include "Utility/Messaging/KOMessageTypes.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Subsystem/KOSkillSubsystem.h"
#include "Data/KODataTableTypes.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/KOAbilitySystemComponent.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "InputCoreTypes.h"
#include "StructUtils/InstancedStruct.h"

// ─────────────────────────────────────────────────────────────────────────────

static FText KeyLabelForSlot(ESkillQuickSlotKey Key)
{
	switch (Key)
	{
	case ESkillQuickSlotKey::Q: return FText::FromString(TEXT("Q"));
	case ESkillQuickSlotKey::E: return FText::FromString(TEXT("E"));
	case ESkillQuickSlotKey::R: return FText::FromString(TEXT("R"));
	case ESkillQuickSlotKey::V: return FText::FromString(TEXT("V"));
	default:                    return FText::FromString(TEXT("?"));
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// 공개 API

void UKOSkillQuickSlotEntryWidget::SetupSlot(ESkillQuickSlotKey InKey)
{
	SlotKey = InKey;
	RefreshKeyLabel();
}

void UKOSkillQuickSlotEntryWidget::AssignSkill(FName InSkillName, FGameplayTag InSkillTag, UTexture2D* InIcon)
{
	SetSlotContent(InSkillName, InSkillTag, InIcon);
}

void UKOSkillQuickSlotEntryWidget::ClearSkill()
{
	UnbindCooldownTracking();

	AssignedSkillName = NAME_None;
	AssignedSkillTag  = FGameplayTag::EmptyTag;
	AssignedIcon      = nullptr;
	CachedInputTag    = FGameplayTag::EmptyTag;

	ApplyIconToImage(nullptr);
	BroadcastChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// 쿨타임 쿼리

float UKOSkillQuickSlotEntryWidget::GetCooldownRemaining() const
{
	if (!CachedCooldownTag.IsValid())
	{
		return 0.f;
	}

	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!ASC)
	{
		return 0.f;
	}

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(CachedCooldownTag);

	TArray<float> Remainders = ASC->GetActiveEffectsTimeRemaining(
		FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(TagContainer));

	return Remainders.IsEmpty() ? 0.f : FMath::Max(0.f, Remainders[0]);
}

float UKOSkillQuickSlotEntryWidget::GetCooldownPercent() const
{
	if (CachedCooldownDuration <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(GetCooldownRemaining() / CachedCooldownDuration, 0.f, 1.f);
}

bool UKOSkillQuickSlotEntryWidget::IsOnCooldown() const
{
	if (!CachedCooldownTag.IsValid())
	{
		return false;
	}

	UAbilitySystemComponent* ASC = GetOwnerASC();
	return ASC && ASC->HasMatchingGameplayTag(CachedCooldownTag);
}

// ─────────────────────────────────────────────────────────────────────────────
// 위젯 생명주기

void UKOSkillQuickSlotEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshKeyLabel();

	if (SkillIconImage && AssignedSkillName.IsNone())
	{
		SkillIconImage->SetVisibility(ESlateVisibility::Hidden);
	}

	// 위젯 재생성 시 이미 스킬이 배정돼 있으면 쿨타임 재구독
	if (HasSkill())
	{
		BindCooldownTracking();
	}

	if (!SlotInputAction)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent);
	if (!EIC)
	{
		return;
	}

	InputBindHandle = EIC->BindAction(
		SlotInputAction,
		ETriggerEvent::Started,
		this,
		&UKOSkillQuickSlotEntryWidget::OnSlotKeyPressed
	).GetHandle();

	InputReleaseBindHandle = EIC->BindAction(
		SlotInputAction,
		ETriggerEvent::Completed,
		this,
		&UKOSkillQuickSlotEntryWidget::OnSlotKeyReleased
	).GetHandle();
}

void UKOSkillQuickSlotEntryWidget::NativeDestruct()
{
	UnbindCooldownTracking();

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent))
		{
			if (InputBindHandle != 0)        EIC->RemoveBindingByHandle(InputBindHandle);
			if (InputReleaseBindHandle != 0) EIC->RemoveBindingByHandle(InputReleaseBindHandle);
		}
	}
	InputBindHandle        = 0;
	InputReleaseBindHandle = 0;

	Super::NativeDestruct();
}

// ─────────────────────────────────────────────────────────────────────────────
// 드래그 시작 (슬롯→슬롯 스왑을 위해)

FReply UKOSkillQuickSlotEntryWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		if (!HasSkill())
		{
			return FReply::Handled();
		}

		if (UKOSkillSubsystem* SkillSubsystem = UKOSkillSubsystem::Get(this))
		{
			SkillSubsystem->SetSkillQuickSlot(SlotKey, NAME_None);
		}

		return FReply::Handled();
	}
	
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && HasSkill())
	{
		FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(
			InMouseEvent, this, EKeys::LeftMouseButton);
		return Reply.NativeReply;
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UKOSkillQuickSlotEntryWidget::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!HasSkill())
	{
		return;
	}

	UKOSkillDragDropOperation* DragOp = UKOSkillDragDropOperation::Create(
		this,
		AssignedSkillName,
		AssignedSkillTag,
		AssignedIcon.Get(),
		DragVisualSize,
		DragVisualOpacity
	);

	if (DragOp)
	{
		DragOp->SourceSlotWidget = this;
	}

	OutOperation = DragOp;
}

// ─────────────────────────────────────────────────────────────────────────────
// 드롭 수신

bool UKOSkillQuickSlotEntryWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation
)
{
	UKOSkillDragDropOperation* SkillOp = Cast<UKOSkillDragDropOperation>(InOperation);
	if (!SkillOp || SkillOp->SkillName.IsNone())
	{
		return false;
	}
	
	UKOSkillSubsystem* SkillSubsystem = UKOSkillSubsystem::Get(this);
	if (!SkillSubsystem)
	{
		return false;
	}

	// ── 슬롯 간 스왑 ──────────────────────────────────────────────
	if (UKOSkillQuickSlotEntryWidget* SrcSlot = SkillOp->SourceSlotWidget.Get())
	{
		if (SrcSlot == this)
		{
			return false;
		}

		const FName TargetOldSkillName = AssignedSkillName;

		SkillSubsystem->SetSkillQuickSlot(SrcSlot->GetSlotKey(), TargetOldSkillName);
		SkillSubsystem->SetSkillQuickSlot(SlotKey, SkillOp->SkillName);

		return true;
	}

	// ── SkillNode에서 새로 배정 ───────────────────────────────────
	return SkillSubsystem->SetSkillQuickSlot(SlotKey, SkillOp->SkillName);
}

// ─────────────────────────────────────────────────────────────────────────────
// BlueprintNativeEvent 기본 구현

void UKOSkillQuickSlotEntryWidget::BP_OnSlotTriggered_Implementation()
{
	// Blueprint에서 오버라이드해 가벼운 눌림 애니메이션을 구현한다.
}

void UKOSkillQuickSlotEntryWidget::BP_OnCooldownBegan_Implementation(float TotalDuration)
{
	// Blueprint에서 오버라이드해 쿨타임 오버레이 애니메이션을 시작한다.
}

void UKOSkillQuickSlotEntryWidget::BP_OnCooldownEnded_Implementation()
{
	// Blueprint에서 오버라이드해 쿨타임 오버레이 애니메이션을 종료한다.
}

// ─────────────────────────────────────────────────────────────────────────────
// 내부 헬퍼

void UKOSkillQuickSlotEntryWidget::SetSlotContent(
	FName InSkillName, FGameplayTag InSkillTag, UTexture2D* InIcon)
{
	UnbindCooldownTracking();

	AssignedSkillName = InSkillName;
	AssignedSkillTag  = InSkillTag;
	AssignedIcon      = InIcon;
	CachedInputTag    = FGameplayTag::EmptyTag;

	// 어빌리티 발동에 쓸 InputTag를 ExRow에서 미리 캐싱한다.
	if (!InSkillName.IsNone())
	{
		if (UKOLoadSubsystem* LS = UKOLoadSubsystem::Get(this))
		{
			if (const FKOSkillExecutionRow* ExRow = LS->FindSkillExecutionRow(InSkillName))
			{
				CachedInputTag = ExRow->InputTag;
			}
		}
	}

	ApplyIconToImage(InIcon);
	BindCooldownTracking();
}

UAbilitySystemComponent* UKOSkillQuickSlotEntryWidget::GetOwnerASC() const
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return nullptr;
	}

	APawn* Pawn = PC->GetPawn();
	if (!Pawn)
	{
		return nullptr;
	}

	return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
}

void UKOSkillQuickSlotEntryWidget::BindCooldownTracking()
{
	if (AssignedSkillName.IsNone())
	{
		return;
	}

	// 1. 어빌리티 CDO에서 CooldownTag를 가져온다
	UKOLoadSubsystem* LS = UKOLoadSubsystem::Get(this);
	if (!LS)
	{
		return;
	}

	const FKOSkillExecutionRow* ExRow = LS->FindSkillExecutionRow(AssignedSkillName);
	if (!ExRow || !ExRow->AbilityClass)
	{
		return;
	}

	const UGameplayAbility* AbilityCDO = ExRow->AbilityClass->GetDefaultObject<UGameplayAbility>();
	if (!AbilityCDO)
	{
		return;
	}

	const FGameplayTagContainer* CooldownTags = AbilityCDO->GetCooldownTags();
	if (!CooldownTags || CooldownTags->IsEmpty())
	{
		return;
	}

	CachedCooldownTag = CooldownTags->First();

	// 2. ASC에 태그 이벤트 등록
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!ASC)
	{
		return;
	}

	CooldownTagDelegateHandle = ASC->RegisterGameplayTagEvent(
		CachedCooldownTag, EGameplayTagEventType::NewOrRemoved
	).AddUObject(this, &UKOSkillQuickSlotEntryWidget::OnCooldownTagChanged);

	// 3. 현재 이미 쿨타임 중이면 즉시 반영
	if (ASC->HasMatchingGameplayTag(CachedCooldownTag))
	{
		FGameplayTagContainer TagContainer;
		TagContainer.AddTag(CachedCooldownTag);

		TArray<TPair<float,float>> TimeAndDuration = ASC->GetActiveEffectsTimeRemainingAndDuration(
			FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(TagContainer));

		CachedCooldownDuration = TimeAndDuration.IsEmpty() ? 0.f : TimeAndDuration[0].Value;
		BP_OnCooldownBegan(CachedCooldownDuration);
	}
}

void UKOSkillQuickSlotEntryWidget::UnbindCooldownTracking()
{
	if (!CooldownTagDelegateHandle.IsValid())
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetOwnerASC())
	{
		ASC->RegisterGameplayTagEvent(CachedCooldownTag, EGameplayTagEventType::NewOrRemoved)
		   .Remove(CooldownTagDelegateHandle);
	}

	CooldownTagDelegateHandle.Reset();
	CachedCooldownTag      = FGameplayTag::EmptyTag;
	CachedCooldownDuration = 0.f;
}

void UKOSkillQuickSlotEntryWidget::OnCooldownTagChanged(const FGameplayTag InTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		// 쿨타임 시작 — 전체 지속 시간 쿼리
		UAbilitySystemComponent* ASC = GetOwnerASC();
		if (ASC)
		{
			FGameplayTagContainer TagContainer;
			TagContainer.AddTag(CachedCooldownTag);

			TArray<float> Durations = ASC->GetActiveEffectsDuration(
				FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(TagContainer));

			CachedCooldownDuration = Durations.IsEmpty() ? 0.f : Durations[0];
		}

		BP_OnCooldownBegan(CachedCooldownDuration);
	}
	else
	{
		// 쿨타임 종료
		CachedCooldownDuration = 0.f;
		BP_OnCooldownEnded();
	}
}

void UKOSkillQuickSlotEntryWidget::OnSlotKeyPressed()
{
	BP_OnSlotTriggered();

	if (!CachedInputTag.IsValid())
	{
		return;
	}

	if (UKOAbilitySystemComponent* KOASC = Cast<UKOAbilitySystemComponent>(GetOwnerASC()))
	{
		KOASC->AbilityInputTagPressed(CachedInputTag);
	}
}

void UKOSkillQuickSlotEntryWidget::OnSlotKeyReleased()
{
	if (!CachedInputTag.IsValid())
	{
		return;
	}

	if (UKOAbilitySystemComponent* KOASC = Cast<UKOAbilitySystemComponent>(GetOwnerASC()))
	{
		KOASC->AbilityInputTagReleased(CachedInputTag);
	}
}

void UKOSkillQuickSlotEntryWidget::RefreshKeyLabel()
{
	if (KeyLabelText)
	{
		KeyLabelText->SetText(KeyLabelForSlot(SlotKey));
	}
}

void UKOSkillQuickSlotEntryWidget::ApplyIconToImage(UTexture2D* InIcon)
{
	if (!SkillIconImage)
	{
		return;
	}

	if (InIcon)
	{
		SkillIconImage->SetBrushFromTexture(InIcon);
		SkillIconImage->SetDesiredSizeOverride(SlotIconSize);
		SkillIconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		SkillIconImage->SetBrushFromTexture(nullptr);
		SkillIconImage->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UKOSkillQuickSlotEntryWidget::BroadcastChanged()
{
	FKOSkillQuickSlotChangedMessage Msg;
	Msg.SlotKey   = SlotKey;
	Msg.SkillName = AssignedSkillName;
	Msg.SkillTag  = AssignedSkillTag;

	Broadcast(
		KOGameplayTags::Data_Message_Skill_QuickSlotChanged,
		FInstancedStruct::Make<FKOSkillQuickSlotChangedMessage>(Msg)
	);
}
