#include "Core/BurningCOREPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Core/UI/BurningCORE_HUD.h"
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

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (TogglePauseAction)
		{
			EnhancedInputComponent->BindAction(TogglePauseAction, ETriggerEvent::Triggered, this, &ABurningCOREPlayerController::OnTogglePause);
		}
	}
}

void ABurningCOREPlayerController::OnTogglePause(const FInputActionValue& Value)
{
	if (ABurningCORE_HUD* BurningHUD = Cast<ABurningCORE_HUD>(GetHUD()))
	{
		BurningHUD->TogglePauseMenu();
	}
}

void ABurningCOREPlayerController::SwitchToSideViewCamera(AActor* CameraTarget)
{
	// Logic to detach from player and attach to target
	UE_LOG(LogBurningCORE, Log, TEXT("Switching to targeted camera view"));
}
