// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "Data/Type/KOSkillTypes.h"
#include "Utility/Messaging/KOGMSInterface.h"
#include "KOSkillQuickSlotWidget.generated.h"

class UKOSkillQuickSlotEntryWidget;
struct FInstancedStruct;

/**
 * Q / E / R / V 슬롯 4개를 묶는 컨테이너 위젯.
 *
 * ── C++ 아이콘 갱신 ─────────────────────────────────────────────
 *  · Data_Message_Skill_QuickSlotChanged 메시지를 C++에서 직접 수신한다.
 *  · 엔트리 위젯이 이미 갱신된 경우(동일 SkillName) 중복 처리를 건너뛴다.
 *  · 외부에서 슬롯 상태가 바뀐 경우 KOLoadSubsystem으로 아이콘을 로드해
 *    해당 엔트리 위젯의 AssignSkill / ClearSkill을 C++에서 직접 호출한다.
 */
UCLASS()
class KARON_API UKOSkillQuickSlotWidget : public UCommonUserWidget, public IKOGMSInterface
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct()  override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UKOSkillQuickSlotEntryWidget> SlotQ;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UKOSkillQuickSlotEntryWidget> SlotE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UKOSkillQuickSlotEntryWidget> SlotR;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UKOSkillQuickSlotEntryWidget> SlotV;

private:
	FGameplayMessageHandle   SlotChangedHandle;
	FGameplayMessageCallback SlotChangedCallback;

	UFUNCTION()
	void HandleSlotChangedMessage(FGameplayTag Channel, const FInstancedStruct& Payload);

	UKOSkillQuickSlotEntryWidget* GetSlotEntryForKey(ESkillQuickSlotKey Key) const;
};