#pragma once

#include "CoreMinimal.h"
#include "KOAnimationTypes.generated.h"

UENUM(BlueprintType)
enum class ELocomotionDirection : uint8
{
	Forward		UMETA(DisplayName = "Forward"),
	Backward	UMETA(DisplayName = "Backward"),
	Left		UMETA(DisplayName = "Left"),
	Right		UMETA(DisplayName = "Right")
};

UENUM(BlueprintType)
enum class EGait : uint8
{
	Walk UMETA(DisplayName = "Walk"),
	Jog UMETA(DisplayName = "Jog"),
};

UENUM(BlueprintType)
enum class EHipFaceDirection : uint8
{
	Forward		UMETA(DisplayName = "Forward"),
	Backward	UMETA(DisplayName = "Backward"),
};

USTRUCT(BlueprintType)
struct FLocomotionDirectionSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ForwardMinAngle; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ForwardMaxAngle;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float BackwardMinAngle;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float BackwardMaxAngle;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float DeadZoneAngle;
};

USTRUCT(BlueprintType)
struct FGaitSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MaxWalkSpeed;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MaxAcceleration;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float BrakingDeceleration;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float BrakingFrictionFactor;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bUseSeparateBrakingFriction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float BrakingFriction;
};

USTRUCT(BlueprintType)
struct FDirectionalAnims
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UAnimSequence* ForwardAnimation; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UAnimSequence* BackwardAnimation; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UAnimSequence* LeftAnimation; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UAnimSequence* RightAnimation; 
};

USTRUCT(BlueprintType)
struct FSpinAnims
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UAnimSequence* ForwardAnimation;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UAnimSequence* BackwardAnimation;
};
