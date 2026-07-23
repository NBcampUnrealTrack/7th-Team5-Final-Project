// Copyright Karon Team 5. All Rights Reserved.

#include "KOAudioSubsystem.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

void UKOAudioSubsystem::PlayPersistentTransitionSound(USoundBase* SoundInside)
{
	if (SoundInside == nullptr || GetWorld() == nullptr)
	{
		return;
	}

	if (TransitionAudioComponent && TransitionAudioComponent->IsPlaying())
	{
		TransitionAudioComponent->Stop();
	}

	// bPersistAcrossLevelTransition을 true로 넘겨야 레벨 전환 시 소유 월드가 바뀌어도 파괴되지 않는다.
	TransitionAudioComponent = UGameplayStatics::SpawnSound2D(
		GetWorld(), SoundInside, 1.f, 1.f, 0.f,
		nullptr, /*bPersistAcrossLevelTransition=*/true, /*bAutoDestroy=*/false
	);

	if (TransitionAudioComponent)
	{
		TransitionAudioComponent->Play();
	}
}
