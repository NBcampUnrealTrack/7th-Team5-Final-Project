// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "KOEnemyDebugUserSettings.generated.h"

/**
 * 에디터 개인설정(Editor Preferences > Karon)에 노출되는 에너미 디버그 세팅입니다.
 * EditorPerProjectUserSettings(개인 Saved/Config)에 저장되므로 사람마다 따로 켜고 끌 수 있습니다.
 */
UCLASS(Config=EditorPerProjectUserSettings, meta = (DisplayName = "Enemy Debug"))
class KARON_API UKOEnemyDebugUserSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	virtual FName GetContainerName() const override { return FName("Editor"); }
	virtual FName GetCategoryName() const override { return FName("Karon"); }

	//에너미/보스 공격 트레이스 디버그 표시 여부 (애님 노티파이 일괄 컨트롤)
	UPROPERTY(Config, EditAnywhere, Category = "Debug")
	bool bShowAttackTraceDebug = false;
};
