#include "Core/BurningCOREPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "BurningCORE.h"

ABurningCOREPlayerController::ABurningCOREPlayerController()
{
	bShowMouseCursor = false;
	DefaultMouseCursor = EMouseCursor::Default;
}

void ABurningCOREPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Add input mapping context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (PlatformerMappingContext)
		{
			Subsystem->AddMappingContext(PlatformerMappingContext, MappingContextPriority);
		}
	}
}

void ABurningCOREPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void ABurningCOREPlayerController::SwitchToSideViewCamera(AActor* CameraTarget)
{
	// Logic to detach from player and attach to target
	UE_LOG(LogBurningCORE, Log, TEXT("Switching to targeted camera view"));
}
