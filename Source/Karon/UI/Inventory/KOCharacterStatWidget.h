// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KOCharacterStatWidget.generated.h"

class UTextBlock;
class UAbilitySystemComponent;
class UKOCombatSet;

/**
 * 캐릭터(플레이어 폰)의 현재 공격력/방어력을 표시하는 위젯.
 * 인벤토리 패널 등에 배치되어 소유 폰의 AbilitySystemComponent에서 값을 읽고,
 * 어트리뷰트 변경 델리게이트를 구독해 실시간으로 갱신한다.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOCharacterStatWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** 외부(패널 등)에서 대상 폰을 명시적으로 지정하고 싶을 때 사용. 없으면 소유 폰을 자동으로 사용. */
    UFUNCTION(BlueprintCallable, Category = "KO|UI|Inventory")
    void SetTargetPawn(APawn* InPawn);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** WBP에 각 위젯 이름으로 배치하면 자동 바인딩. (없으면 해당 항목만 표시 생략) */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> AttackPowerText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> DefenseText;

private:
    void InitializeFromPawn(APawn* Pawn);
    void BindToAbilitySystem(UAbilitySystemComponent* ASC);
    void UnbindFromAbilitySystem();

    void RefreshCombat();

    UFUNCTION()
    void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

    UFUNCTION()
    void OnAttackPowerChanged(float OldValue, float NewValue);

    UFUNCTION()
    void OnDefenseChanged(float OldValue, float NewValue);

    UPROPERTY(Transient)
    TObjectPtr<UAbilitySystemComponent> CachedASC;

    UPROPERTY(Transient)
    TObjectPtr<UKOCombatSet> CachedCombatSet;
};