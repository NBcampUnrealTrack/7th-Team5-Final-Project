#include "KOGA_SnowBall.h"

#include "EnhancedInputComponent.h"
#include "AbilitySystem/Ability/AbilityTask/AbilityTask_Tick.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UKOGA_SnowBall::UKOGA_SnowBall()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UKOGA_SnowBall::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	ACharacter* Character = GetAvatarCharacter();
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	CurrentScale = .1f;
	SteerInput = 0.f;
	
	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (!MoveComp)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	OriginalSpeed = MoveComp->MaxWalkSpeed; 
	
	FTransform SpawnTransform = Character->GetActorTransform();
	SnowballActor = GetWorld()->SpawnActor<AActor>(SnowballClass, SpawnTransform);
	if (!SnowballActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// SnowballActor->OnHit.AddDynamic(this, &UGA_Snowball::OnSnowballHit);
	// SnowballActor->MoveIgnoreActorAdd(Avatar); // 캐릭터 밀림 방지
	
	UAbilityTask_Tick* TickTask = UAbilityTask_Tick::CreateTickTask(this);
	TickTask->OnTick.AddDynamic(this, &UKOGA_SnowBall::OnSnowballTick);
	TickTask->ReadyForActivation();
}

void UKOGA_SnowBall::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 이동속도 복구
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		UCharacterMovementComponent* MoveComp =
			Avatar->FindComponentByClass<UCharacterMovementComponent>();
		if (MoveComp)
		{
			MoveComp->MaxWalkSpeed = OriginalSpeed;
		}
	}

	if (SnowballActor)
	{
		SnowballActor->Destroy();
		SnowballActor = nullptr;
	}

	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_SnowBall::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UKOGA_SnowBall::OnSnowballTick(float DeltaTime)
{
	if (!SnowballActor) return;

	ACharacter* Character = GetAvatarCharacter();
	if (!Character) return;
	
	// 1. 스티어링 입력 직접 읽기
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(Character->InputComponent))
	{
		SteerInput = EIC->GetBoundActionValue(SteerAction).Get<float>();
	}
	
	// 1. 스티어링 - 캐릭터 회전
	if (!FMath::IsNearlyZero(SteerInput))
	{
		FRotator NewRot = Character->GetActorRotation();
		NewRot.Yaw += SteerInput * SteerSpeed * DeltaTime;
		Character->SetActorRotation(NewRot);
	}

	// 2. 지면 Z 계산 - 캡슐 발 위치 기준 LineTrace
	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	float HalfHeight = Capsule ? Capsule->GetScaledCapsuleHalfHeight() : 90.f;
	FVector TraceStart = Character->GetActorLocation() - FVector(0, 0, HalfHeight);

	FHitResult GroundHit;
	GetWorld()->LineTraceSingleByChannel(GroundHit, TraceStart,
		TraceStart - FVector(0, 0, 500.f), ECC_WorldStatic);

	float GroundZ = GroundHit.bBlockingHit ? GroundHit.ImpactPoint.Z : TraceStart.Z;
	
	// 3. 지면 법선으로 눈덩이 정렬
	FVector GroundNormal = GroundHit.bBlockingHit ?
		GroundHit.ImpactNormal : FVector::UpVector;

	// 4. 캐릭터 앞쪽 위치 - 크기에 따라 거리 증가
	FVector Offset = Character->GetActorForwardVector() * (BaseRadius * CurrentScale);
	FVector NewLocation = Character->GetActorLocation() + Offset;
	NewLocation.Z = GroundZ + (BaseRadius * CurrentScale);

	SnowballActor->SetActorLocation(NewLocation, true); // bSweep=true
	
	// 5. 지면 법선 기반 회전 + Pitch 굴림 회전 합산
	float RollDelta = (100.f / (BaseRadius * CurrentScale)) * (180.f / PI) * DeltaTime;
	FRotator GroundAlignedRot = FRotationMatrix::MakeFromZX(
		GroundNormal, Character->GetActorForwardVector()).Rotator();
	GroundAlignedRot.Pitch += RollDelta;
	SnowballActor->SetActorRotation(GroundAlignedRot);
	
	// 6. 크기 증가
	CurrentScale = FMath::Min(CurrentScale + GrowthRate * DeltaTime, MaxScale);
	SnowballActor->SetActorScale3D(FVector(CurrentScale));
	
	// 7. 이동속도 증가
	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (MoveComp)
	{
		float SpeedBonus = (CurrentScale - 1.f) * BaseSpeedBonus;
		MoveComp->MaxWalkSpeed = OriginalSpeed + SpeedBonus;
	}

	// 8. 최대 크기 도달 시 자동 발동
	if (FMath::IsNearlyEqual(CurrentScale, MaxScale, 0.01f))
	{
		OnSnowballHit(nullptr);
	}
	
	Character->LaunchCharacter(
		Character->GetActorForwardVector() * SteerSpeed * DeltaTime, 
		false, false
	);
}


void UKOGA_SnowBall::OnSnowballHit(AActor* HitActor)
{
}


