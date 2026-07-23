#include "UI/RootLayout/KOTitleRootLayout.h"
#include "UI/KOUISubsystem.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"

#include "Widgets/CommonActivatableWidgetContainer.h"

void UKOTitleRootLayout::NativeConstruct()
{
	Super::NativeConstruct();
	
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		return;
	}

	UKOUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UKOUISubsystem>();
	if (UISubsystem == nullptr)
	{
		return;
	}
	// 1. 메인 메뉴용 태그와 매핑하여 내부 컨테이너 컴포넌트를 등록!
	if (TitleLayer)
	{
		UISubsystem->RegisterPrimaryLayout(KOGameplayTags::UI_Layer_Menu, TitleLayer);
	}

	// 2. 옵션 팝업 등 최상위 모달 위젯을 위한 레이어 등록.
	if (ModalLayer)
	{
		UISubsystem->RegisterPrimaryLayout(KOGameplayTags::UI_Layer_Modal, ModalLayer);
	}
}
