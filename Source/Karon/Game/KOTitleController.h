#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KOTitleController.generated.h"

/**
 * 타이틀 전용 컨트롤러
 */
UCLASS()
class KARON_API AKOTitleController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
};
