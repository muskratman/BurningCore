#include "Systems/CheckpointActor.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Core/BurningCOREGameMode.h"
#include "Core/BurningCOREGameInstance.h"

ACheckpointActor::ACheckpointActor()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
	TriggerZone->SetCollisionProfileName(TEXT("Trigger"));
	RootComponent = TriggerZone;

	// Niagara component normally attached here
}

void ACheckpointActor::BeginPlay()
{
	Super::BeginPlay();
}

void ACheckpointActor::Interact_Implementation(AActor* Interactor)
{
	bIsActive = true;
	
	// Register with Game Mode
	if (ABurningCOREGameMode* GM = Cast<ABurningCOREGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->RegisterCheckpoint(this);
	}
	
	// Save game
	if (UBurningCOREGameInstance* GI = Cast<UBurningCOREGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SaveProgress();
	}
	
	// Play activation VFX
}

bool ACheckpointActor::CanInteract_Implementation(AActor* Interactor) const
{
	return true; // Always can interact in this basic implementation
}

FText ACheckpointActor::GetInteractionPrompt_Implementation() const
{
	return FText::Format(NSLOCTEXT("BurningCORE", "CheckpointPrompt", "Rest at {0}"), CheckpointName);
}
