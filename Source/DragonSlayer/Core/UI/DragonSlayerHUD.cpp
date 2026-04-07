#include "Core/UI/DragonSlayerHUD.h"
#include "UI/PauseMenu/DeveloperSettingsWidget.h"
#include "UI/PauseMenu/PauseWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
void ApplyMenuInputMode(APlayerController* PlayerController, UUserWidget* FocusWidget)
{
	if (!PlayerController || !FocusWidget)
	{
		return;
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
	InputMode.SetHideCursorDuringCapture(false);

	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
}

void RestoreGameInputMode(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	FInputModeGameOnly InputMode;
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = false;
}
}

ADragonSlayerHUD::ADragonSlayerHUD()
{
	static ConstructorHelpers::FClassFinder<UPauseWidget> PauseWidgetClassFinder(
		TEXT("/Game/Blueprints/Widgets/WBP_PauseMenu"));
	if (PauseWidgetClassFinder.Succeeded())
	{
		PauseWidgetClass = PauseWidgetClassFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UDeveloperSettingsWidget> DeveloperSettingsWidgetClassFinder(
		TEXT("/CookieBrosPlatformer/Blueprints/UI/WBP_DeveloperSettings"));
	if (DeveloperSettingsWidgetClassFinder.Succeeded())
	{
		DeveloperSettingsWidgetClass = DeveloperSettingsWidgetClassFinder.Class;
	}
	else
	{
		DeveloperSettingsWidgetClass = UDeveloperSettingsWidget::StaticClass();
	}
}

void ADragonSlayerHUD::TogglePauseMenu()
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

void ADragonSlayerHUD::ShowPauseMenu()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	if (DeveloperSettingsWidgetInstance && DeveloperSettingsWidgetInstance->IsInViewport())
	{
		DeveloperSettingsWidgetInstance->RemoveFromParent();
	}

	if (!PauseWidgetInstance && PauseWidgetClass)
	{
		PauseWidgetInstance = CreateWidget<UPauseWidget>(PC, PauseWidgetClass);
	}

	if (PauseWidgetInstance && !PauseWidgetInstance->IsInViewport())
	{
		PauseWidgetInstance->AddToViewport(10); // High Z-order to be on top
		ApplyMenuInputMode(PC, PauseWidgetInstance);
		PC->SetPause(true);
	}
}

void ADragonSlayerHUD::HidePauseMenu()
{
	if (PauseWidgetInstance && PauseWidgetInstance->IsInViewport())
	{
		PauseWidgetInstance->RemoveFromParent();

		if (APlayerController* PC = GetOwningPlayerController())
		{
			RestoreGameInputMode(PC);
			PC->SetPause(false);
		}
	}
}

void ADragonSlayerHUD::ToggleDeveloperSettingsWidget()
{
	if (DeveloperSettingsWidgetInstance && DeveloperSettingsWidgetInstance->IsInViewport())
	{
		HideDeveloperSettingsWidget();
	}
	else
	{
		ShowDeveloperSettingsWidget();
	}
}

void ADragonSlayerHUD::ShowDeveloperSettingsWidget()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	if (PauseWidgetInstance && PauseWidgetInstance->IsInViewport())
	{
		PauseWidgetInstance->RemoveFromParent();
	}

	if (!DeveloperSettingsWidgetInstance && DeveloperSettingsWidgetClass)
	{
		DeveloperSettingsWidgetInstance = CreateWidget<UDeveloperSettingsWidget>(PC, DeveloperSettingsWidgetClass);
	}

	if (DeveloperSettingsWidgetInstance && !DeveloperSettingsWidgetInstance->IsInViewport())
	{
		DeveloperSettingsWidgetInstance->AddToViewport(10);
		ApplyMenuInputMode(PC, DeveloperSettingsWidgetInstance);
		PC->SetPause(true);
	}
}

void ADragonSlayerHUD::HideDeveloperSettingsWidget()
{
	if (DeveloperSettingsWidgetInstance && DeveloperSettingsWidgetInstance->IsInViewport())
	{
		DeveloperSettingsWidgetInstance->RemoveFromParent();

		if (APlayerController* PC = GetOwningPlayerController())
		{
			RestoreGameInputMode(PC);
			PC->SetPause(false);
		}
	}
}
