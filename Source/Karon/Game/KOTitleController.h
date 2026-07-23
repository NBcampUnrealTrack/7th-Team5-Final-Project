#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KOTitleController.generated.h"

class USoundMix;
/**
 * 타이틀 전용 컨트롤러.
 * 위젯을 직접 들지 않는다. 루트 레이아웃/타이틀 메뉴는 UISubsystem이 UKOUISettings를 통해 관리.
 */
UCLASS()
class KARON_API AKOTitleController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Autio Settings")
	TObjectPtr<USoundMix> DefaultSoundMix;
};
