#include "MainMenuGameMode.h"
#include "MainMenuPlayerController.h"
#include "MainMenuHUD.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	PlayerControllerClass = AMainMenuPlayerController::StaticClass();
	HUDClass = AMainMenuHUD::StaticClass();
	DefaultPawnClass = nullptr;
}
