#include "UI/PlatformerDefeatWidget.h"

#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UPlatformerDefeatWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Restart)
	{
		Btn_Restart->OnClicked.AddDynamic(this, &UPlatformerDefeatWidget::OnRestartClicked);
	}

	if (Btn_MainMenu)
	{
		Btn_MainMenu->OnClicked.AddDynamic(this, &UPlatformerDefeatWidget::OnMainMenuClicked);
	}

	if (Btn_Exit)
	{
		Btn_Exit->OnClicked.AddDynamic(this, &UPlatformerDefeatWidget::OnExitClicked);
	}
}

void UPlatformerDefeatWidget::CloseDefeatMenu()
{
	RemoveFromParent();

	if (APlayerController* OwningPlayerController = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		OwningPlayerController->SetInputMode(InputMode);
		OwningPlayerController->bShowMouseCursor = false;
		OwningPlayerController->SetPause(false);
	}
}

void UPlatformerDefeatWidget::OnRestartClicked()
{
	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (CurrentLevelName.IsEmpty())
	{
		return;
	}

	CloseDefeatMenu();
	UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
}

void UPlatformerDefeatWidget::OnMainMenuClicked()
{
	if (MainMenuLevelName.IsNone())
	{
		return;
	}

	CloseDefeatMenu();
	UGameplayStatics::OpenLevel(this, MainMenuLevelName);
}

void UPlatformerDefeatWidget::OnExitClicked()
{
	CloseDefeatMenu();
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
