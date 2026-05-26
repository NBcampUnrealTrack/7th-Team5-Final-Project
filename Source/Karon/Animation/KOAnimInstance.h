#pragma once

#include "CoreMinimal.h"
#include "KOAnimationTypes.h"
#include "Animation/AnimInstance.h"
#include "KOAnimInstance.generated.h"

class UCharacterTrajectoryComponent;
class AKOHeroCharacter;
class UCharacterMovementComponent; 

UCLASS()
class KARON_API UKOAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	void ReceiveGait(EGait NewGait);
	
protected:
	// References 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Reference")
	TObjectPtr<AKOHeroCharacter> CachedCharacter;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Reference")
	TObjectPtr<UCharacterTrajectoryComponent> CachedTrajectoryComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Reference")
	TObjectPtr<UCharacterMovementComponent> CachedMovementComponent;
	
protected:
	// Transform Variables 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Location Data")
	FVector WorldLocation;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rotation Data")
	FRotator WorldRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rotation Data")
	float ActorYaw; 
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rotation Data")
	float PreviousActorYaw; 
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rotation Data")
	float ActorYawDelta; 
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rotation Data")
	float LeanAngle; 
	
protected:
	// Velocity Variables 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Velocity Data")
	FVector PreviousCharacterVelocity2D;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Velocity Data")
	FVector CharacterVelocity;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Velocity Data")
	FVector CharacterVelocity2D;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Velocity Data")
	FVector PhysicalAcceleration2D;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Velocity Data")
	FVector RelativeAccelerationAmount;
	
protected:
	// Acceleration Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Acceleration Data")
	FVector CharacterAcceleration;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Acceleration Data")
	FVector CharacterAcceleration2D;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Acceleration Data")
	bool bHasAcceleration;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rotation Data")
	float LocomotionAngle; 
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rotation Data")
	ELocomotionDirection LocomotionDirection;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rotation Data")
	ELocomotionDirection PreviousLocomotionDirection;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rotation Data")
	EHipFaceDirection HipFaceDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Rotation Data")
	TMap<ELocomotionDirection, EHipFaceDirection>DesiredHipFacing;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gait")
	EGait InComingGait;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gait")
	EGait CurrentGait;
};
