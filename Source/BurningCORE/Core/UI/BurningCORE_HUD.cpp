#include "Core/UI/BurningCORE_HUD.h"
#include "Variant_SideScrolling/UI/PauseMenu/BurningCORE_PauseWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

ABurningCORE_HUD::ABurningCORE_HUD()
{
}

void ABurningCORE_HUD::TogglePauseMenu()
{
	if (PauseWidgetInstance && PauseWidgetInstance->IsInViewport())
	{
		HidePauseMenu();
	}
	else
	{
		ShowPauseMenu();
	}
}

void ABurningCORE_HUD::ShowPauseMenu()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	if (!PauseWidgetInstance && PauseWidgetClass)
	{
		PauseWidgetInstance = CreateWidget<UBurningCORE_PauseWidget>(PC, PauseWidgetClass);
	}

	if (PauseWidgetInstance && !PauseWidgetInstance->IsInViewport())
	{
		PauseWidgetInstance->AddToViewport(10); // High Z-order to be on top

		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(PauseWidgetInstance->TakeWidget());
		InputMode.SetHideCursorDuringCapture(false);
		
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
		PC->SetPause(true);
	}
}

void ABurningCORE_HUD::HidePauseMenu()
{
	if (PauseWidgetInstance && PauseWidgetInstance->IsInViewport())
	{
		PauseWidgetInstance->RemoveFromParent();

		if (APlayerController* PC = GetOwningPlayerController())
		{
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = false;
			PC->SetPause(false);
		}
	}
}
