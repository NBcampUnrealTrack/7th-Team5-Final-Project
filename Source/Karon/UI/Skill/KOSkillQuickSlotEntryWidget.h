// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "Data/Type/KOSkillTypes.h"
#include "Utility/Messaging/KOGMSInterface.h"
#include "KOSkillQuickSlotEntryWidget.generated.h"

class UImage;
class UTextBlock;
class UInputAction;
class UDragDropOperation;
class UTexture2D;
class UAbilitySystemComponent;

/**
 * 스킬 퀵슬롯 한 칸 (Q / E / R / V 중 하나).
 *
 * ── 드래그 드롭 ─────────────────────────────────────────────────
 *  · SkillNodeWidget 또는 다른 슬롯에서 드래그해 스킬을 배정·교환한다.
 *  · 슬롯 간 드래그 시 두 슬롯의 스킬이 교환된다.
 *
 * ── 키 입력 시각 피드백 ──────────────────────────────────────────
 *  · SlotInputAction이 연결되면 해당 키 입력 시 BP_OnSlotTriggered를 호출한다.
 *
 * ── 쿨타임 연동 (GAS CooldownTag 기반) ──────────────────────────
 *  · 스킬이 배정되면 GAS에서 해당 어빌리티의 CooldownTag를 자동 구독한다.
 *  · 쿨타임 시작/종료 시 BP_OnCooldownBegan / BP_OnCooldownEnded가 호출된다.
 *  · BP에서 GetCooldownPercent()를 Tick 또는 타이머로 폴링해 프로그레스바를 갱신한다.
 *
 * ── HUD 갱신 ────────────────────────────────────────────────────
 *  · 배정이 바뀔 때마다 FKOSkillQuickSlotChangedMessage를 브로드캐스트한다.
 *  · KOInGameHUD::BP_OnSkillQuickSlotChanged가 이를 수신한다.
 */
UCLASS()
class KARON_API UKOSkillQuickSlotEntryWidget : public UCommonUserWidget, public IKOGMSInterface
{
	GENERATED_BODY()

public:
	/** 컨테이너(KOSkillQuickSlotWidget)에서 슬롯 키를 지정할 때 호출 */
	void SetupSlot(ESkillQuickSlotKey InKey);

	/** 드래그 드롭 외부에서 스킬을 직접 배정할 때 사용 */
	void AssignSkill(FName InSkillName, FGameplayTag InSkillTag, UTexture2D* InIcon);

	void ClearSkill();

	ESkillQuickSlotKey GetSlotKey()           const { return SlotKey; }
	FName              GetAssignedSkillName() const { return AssignedSkillName; }
	FGameplayTag       GetAssignedSkillTag()  const { return AssignedSkillTag; }
	UTexture2D*        GetAssignedIcon()      const { return AssignedIcon.Get(); }
	bool               HasSkill()             const { return !AssignedSkillName.IsNone(); }

	// ── 쿨타임 쿼리 (BP Tick에서 프로그레스바 갱신용) ──────────────
	/** 남은 쿨타임(초). 쿨타임 중이 아니면 0. */
	UFUNCTION(BlueprintPure, Category = "SkillQuickSlot|Cooldown")
	float GetCooldownRemaining() const;

	/** 전체 쿨타임 지속 시간(초). */
	UFUNCTION(BlueprintPure, Category = "SkillQuickSlot|Cooldown")
	float GetCooldownDuration() const { return CachedCooldownDuration; }

	/** 쿨타임 진행률 (0.0 = 대기 가능, 1.0 = 막 발동). */
	UFUNCTION(BlueprintPure, Category = "SkillQuickSlot|Cooldown")
	float GetCooldownPercent() const;

	UFUNCTION(BlueprintPure, Category = "SkillQuickSlot|Cooldown")
	bool IsOnCooldown() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct()  override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void   NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool   NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	// ── 키 입력 피드백 ──────────────────────────────────────────────
	/** 배정된 키가 입력될 때 호출. Blueprint에서 오버라이드해 가벼운 애니메이션/이펙트를 구현한다. */
	UFUNCTION(BlueprintNativeEvent, Category = "SkillQuickSlot")
	void BP_OnSlotTriggered();

	// ── GAS 쿨타임 콜백 ─────────────────────────────────────────────
	/** 어빌리티가 쿨타임에 들어갔을 때 호출. TotalDuration은 GA의 CooldownDuration. */
	UFUNCTION(BlueprintNativeEvent, Category = "SkillQuickSlot|Cooldown")
	void BP_OnCooldownBegan(float TotalDuration);

	/** 쿨타임이 끝났을 때 호출. */
	UFUNCTION(BlueprintNativeEvent, Category = "SkillQuickSlot|Cooldown")
	void BP_OnCooldownEnded();

	// ── 바인딩 위젯 ────────────────────────────────────────────────
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> SkillIconImage;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> CoolDownOverlay;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CoolDownText;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> KeyLabelText;

	// ── 에디터 설정 ────────────────────────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "SkillQuickSlot|Visual")
	FVector2D SlotIconSize = FVector2D(64.f, 64.f);

	UPROPERTY(EditDefaultsOnly, Category = "SkillQuickSlot|Drag")
	FVector2D DragVisualSize = FVector2D(64.f, 64.f);

	UPROPERTY(EditDefaultsOnly, Category = "SkillQuickSlot|Drag")
	float DragVisualOpacity = 0.85f;

	/** CoolDownOverlay 다이나믹 머티리얼에서 쿨타임 진행률(0=사용 가능, 1=막 발동)을 채워 넣을 스칼라 파라미터 이름. */
	UPROPERTY(EditDefaultsOnly, Category = "SkillQuickSlot|Cooldown")
	FName CooldownPercentParamName = TEXT("Percent");

	/** 이 슬롯에 대응하는 InputAction (BP 에디터에서 Q/E/R/V 액션을 지정) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillQuickSlot|Input")
	TObjectPtr<const UInputAction> SlotInputAction = nullptr;

private:
	// ── 슬롯 상태 ──────────────────────────────────────────────────
	ESkillQuickSlotKey         SlotKey           = ESkillQuickSlotKey::Q;
	FName                      AssignedSkillName;
	FGameplayTag               AssignedSkillTag;
	TWeakObjectPtr<UTexture2D> AssignedIcon;

	// ── 쿨타임 추적 ────────────────────────────────────────────────
	FGameplayTag    CachedCooldownTag;
	float           CachedCooldownDuration = 0.f;
	FDelegateHandle CooldownTagDelegateHandle;

	// ── GA 입력 태그 ────────────────────────────────────────────────
	FGameplayTag CachedInputTag;

	// ── 입력 바인딩 ────────────────────────────────────────────────
	uint32 InputBindHandle        = 0;
	uint32 InputReleaseBindHandle = 0;

	// ── 내부 헬퍼 ──────────────────────────────────────────────────
	/** 데이터 설정 + 아이콘 갱신 + 쿨타임 재바인딩 + 메시지 브로드캐스트를 일괄 처리 */
	void SetSlotContent(FName InSkillName, FGameplayTag InSkillTag, UTexture2D* InIcon);

	UAbilitySystemComponent* GetOwnerASC() const;
	void BindCooldownTracking();
	void UnbindCooldownTracking();
	void OnCooldownTagChanged(const FGameplayTag InTag, int32 NewCount);

	void ApplyIconToImage(UTexture2D* InIcon);
	void OnSlotKeyPressed();
	void OnSlotKeyReleased();
	void RefreshKeyLabel();
	void BroadcastChanged();

	/** CoolDownOverlay(진행률)/CoolDownText(남은 시간)를 현재 쿨타임 상태로 갱신. */
	void RefreshCooldownVisual();
};