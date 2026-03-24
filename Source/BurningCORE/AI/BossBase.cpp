#include "AI/BossBase.h"
#include "Kismet/GameplayStatics.h"
#include "Core/BurningCOREGameMode.h"

ABossBase::ABossBase()
{
	// Bosses generally have much larger StaggerThresholds or are immune to basic hit reactions
	StaggerThreshold = 50.0f;
}

void ABossBase::BeginPlay()
{
	Super::BeginPlay();

	// Notify GameMode we exist and are active
	if (ABurningCOREGameMode* GM = Cast<ABurningCOREGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->ActivateBossEncounter(this);
	}
}

void ABossBase::TransitionToPhase_Implementation(int32 NextPhase)
{
	CurrentPhase = NextPhase;
	// Handle model/material/moveset changes in blueprint overriding this native event
}
