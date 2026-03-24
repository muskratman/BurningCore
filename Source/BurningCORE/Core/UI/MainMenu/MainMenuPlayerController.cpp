#include "MainMenuPlayerController.h"

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	
	FInputModeUIOnly InputModeData;
	SetInputMode(InputModeData);
}
