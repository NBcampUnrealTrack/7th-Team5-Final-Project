#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "KOBossSmokeData.generated.h"

class UNiagaraSystem;
 
// VFX 채널
USTRUCT(BlueprintType)
struct FKOBossVFXChannel
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraSystem> Effect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> Sockets;
};

// 분출 스텝
USTRUCT(BlueprintType)
struct FKOBossSmokePuffStep
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ChannelIndex = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SocketIndex = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Duration = 0.3f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpawnRate = 20.f;
};

// 상태별 패턴
USTRUCT(BlueprintType)
struct FKOBossSmokePattern
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FKOBossSmokePuffStep> Steps;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float StepInterval = 0.4f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PauseInterval = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bLoop = true;
};

UCLASS(BlueprintType)
class KARON_API UKOBossSmokeData : public UDataAsset
{
	GENERATED_BODY()

public:
	// VFX 채널 배열
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TArray<FKOBossVFXChannel> VFXChannels;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Pattern")
	FKOBossSmokePattern IdlePattern;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Pattern")
	FKOBossSmokePattern GimmickPattern;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Pattern")
	FKOBossSmokePattern DashPattern;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Pattern")
	FKOBossSmokePattern GroggyPattern;
};
