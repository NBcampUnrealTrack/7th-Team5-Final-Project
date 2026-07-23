// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Skill/KOSkillQuickSlotWidget.h"
#include "UI/Skill/KOSkillQuickSlotEntryWidget.h"

#include "AbilitySystem/Tag/Data/KOGameplayTags_Data.h"
#include "Utility/Messaging/KOMessageTypes.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Subsystem/KOSkillSubsystem.h"

#include "StructUtils/InstancedStruct.h"

// ─────────────────────────────────────────────────────────────────────────────

void UKOSkillQuickSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SlotQ) SlotQ->SetupSlot(ESkillQuickSlotKey::Q);
	if (SlotE) SlotE->SetupSlot(ESkillQuickSlotKey::E);
	if (SlotR) SlotR->SetupSlot(ESkillQuickSlotKey::R);
	if (SlotV) SlotV->SetupSlot(ESkillQuickSlotKey::V);

	SlotChangedCallback.BindDynamic(this, &UKOSkillQuickSlotWidget::HandleSlotChangedMessage);
	SlotChangedHandle = Subscribe(
		KOGameplayTags::Data_Message_Skill_QuickSlotChanged,
		SlotChangedCallback);
	
	if (UKOSkillSubsystem* SkillSubsystem = UKOSkillSubsystem::Get(this))
	{
		const ESkillQuickSlotKey SlotKeys[] =
		{
			ESkillQuickSlotKey::Q,
			ESkillQuickSlotKey::E,
			ESkillQuickSlotKey::R,
			ESkillQuickSlotKey::V
		};

		for (const ESkillQuickSlotKey SlotKey : SlotKeys)
		{
			const FName SkillName = SkillSubsystem->GetSkillQuickSlot(SlotKey);

			FKOSkillQuickSlotChangedMessage Msg;
			Msg.SlotKey = SlotKey;
			Msg.SkillName = SkillName;
			Msg.SkillTag = FGameplayTag::EmptyTag;

			if (!SkillName.IsNone())
			{
				if (UKOLoadSubsystem* LS = UKOLoadSubsystem::Get(this))
				{
					if (const FKOSkillRow* Row = LS->FindSkillRow(SkillName))
					{
						Msg.SkillTag = Row->SkillTag;
					}
				}
			}

			HandleSlotChangedMessage(
				KOGameplayTags::Data_Message_Skill_QuickSlotChanged,
				FInstancedStruct::Make<FKOSkillQuickSlotChangedMessage>(Msg)
			);
		}
	}
}

void UKOSkillQuickSlotWidget::NativeDestruct()
{
	Unsubscribe(SlotChangedHandle);
	SlotChangedCallback.Clear();

	Super::NativeDestruct();
}

// ─────────────────────────────────────────────────────────────────────────────

UKOSkillQuickSlotEntryWidget* UKOSkillQuickSlotWidget::GetSlotEntryForKey(ESkillQuickSlotKey Key) const
{
	switch (Key)
	{
	case ESkillQuickSlotKey::Q: return SlotQ;
	case ESkillQuickSlotKey::E: return SlotE;
	case ESkillQuickSlotKey::R: return SlotR;
	case ESkillQuickSlotKey::V: return SlotV;
	default:                    return nullptr;
	}
}

void UKOSkillQuickSlotWidget::HandleSlotChangedMessage(
	FGameplayTag Channel, const FInstancedStruct& Payload)
{
	const FKOSkillQuickSlotChangedMessage* Msg = Payload.GetPtr<FKOSkillQuickSlotChangedMessage>();
	if (!Msg)
	{
		return;
	}

	UKOSkillQuickSlotEntryWidget* SlotEntry = GetSlotEntryForKey(Msg->SlotKey);
	if (!SlotEntry)
	{
		return;
	}

	// 엔트리 위젯이 이미 동일한 스킬로 갱신돼 있으면 아무것도 하지 않는다.
	// (드래그-드롭 등 엔트리 자체가 BroadcastChanged를 보낸 경우)
	if (SlotEntry->GetAssignedSkillName() == Msg->SkillName)
	{
		return;
	}

	// 외부에서 슬롯 상태가 변경된 경우 — 아이콘을 C++에서 로드해 직접 배정한다.
	if (Msg->SkillName.IsNone())
	{
		SlotEntry->ClearSkill();
	}
	else
	{
		UTexture2D* Icon = nullptr;
		if (UKOLoadSubsystem* LS = UKOLoadSubsystem::Get(this))
		{
			Icon = LS->ResolveSkillIcon(Msg->SkillName);
		}

		SlotEntry->AssignSkill(Msg->SkillName, Msg->SkillTag, Icon);
	}
}