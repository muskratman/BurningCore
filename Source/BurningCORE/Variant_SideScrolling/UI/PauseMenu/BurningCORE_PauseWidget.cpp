#include "Variant_SideScrolling/UI/PauseMenu/BurningCORE_PauseWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Core/UI/BurningCORE_HUD.h"

void UBurningCORE_PauseWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Resume)
	{
		Btn_Resume->OnClicked.AddDynamic(this, &UBurningCORE_PauseWidget::OnResumeClicked);
	}

	if (Btn_MainMenu)
	{
		Btn_MainMenu->OnClicked.AddDynamic(this, &UBurningCORE_PauseWidget::OnMainMenuClicked);
	}
	
	if (Btn_Settings)
	{
		Btn_Settings->OnClicked.AddDynamic(this, &UBurningCORE_PauseWidget::OnSettingsClicked);
	}

	if (Btn_DevSettings)
	{
		Btn_DevSettings->OnClicked.AddDynamic(this, &UBurningCORE_PauseWidget::OnDevSettingsClicked);
	}

	if (Btn_Exit)
	{
		Btn_Exit->OnClicked.AddDynamic(this, &UBurningCORE_PauseWidget::OnExitClicked);
	}
}

void UBurningCORE_PauseWidget::OnResumeClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ABurningCORE_HUD* HUD = Cast<ABurningCORE_HUD>(PC->GetHUD()))
		{
			HUD->HidePauseMenu();
		}
	}
}

void UBurningCORE_PauseWidget::OnMainMenuClicked()
{
	// Hide pause before transitioning to prevent input lock issues
	OnResumeClicked();
	UGameplayStatics::OpenLevel(this, FName("MainMenu"));
}

void UBurningCORE_PauseWidget::OnSettingsClicked()
{
	// TBD: Toggle visibility of a settings panel/widget
	UE_LOG(LogTemp, Warning, TEXT("Settings clicked - Placeholder"));
}

void UBurningCORE_PauseWidget::OnDevSettingsClicked()
{
	// TBD: Toggle visibility of DevSettings panel
	UE_LOG(LogTemp, Warning, TEXT("DevSettings clicked - Placeholder"));
}

void UBurningCORE_PauseWidget::OnExitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
