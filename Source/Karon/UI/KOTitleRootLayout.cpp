#include "KOTitleRootLayout.h"
#include "KOUISubsystem.h"

#include "Widgets/CommonActivatableWidgetContainer.h"

void UKOTitleRootLayout::NativeConstruct()
{
	Super::NativeConstruct();
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (LocalPlayer)
	{
		UKOUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UKOUISubsystem>();
		if (UISubsystem)
		{
			// 1. 메인 메뉴용 태그와 매핑하여 내부 컨테이너 컴포넌트를 등록!
			if (TitleLayer)
			{
				FGameplayTag TitleTag = FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.Menu"));
				UISubsystem->RegisterPrimaryLayout(TitleTag, TitleLayer);
			}
		}
	}
}
