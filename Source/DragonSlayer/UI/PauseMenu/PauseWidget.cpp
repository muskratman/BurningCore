#include "UI/PauseMenu/PauseWidget.h"
#include "Core/UI/DragonSlayerHUD.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UPauseWidget::NativeOnInitialized() {
  Super::NativeOnInitialized();

  if (Btn_Resume) {
    Btn_Resume->OnClicked.AddDynamic(this, &UPauseWidget::OnResumeClicked);
  }

  if (Btn_MainMenu) {
    Btn_MainMenu->OnClicked.AddDynamic(this, &UPauseWidget::OnMainMenuClicked);
  }

  if (Btn_Settings) {
    Btn_Settings->OnClicked.AddDynamic(this, &UPauseWidget::OnSettingsClicked);
  }

  if (Btn_DevSettings) {
    Btn_DevSettings->OnClicked.AddDynamic(this, &UPauseWidget::OnDevSettingsClicked);
  }

  if (Btn_Exit) {
    Btn_Exit->OnClicked.AddDynamic(this, &UPauseWidget::OnExitClicked);
  }
}

void UPauseWidget::OnResumeClicked() {
  if (APlayerController *PC = GetOwningPlayer()) {
    if (ADragonSlayerHUD *HUD = Cast<ADragonSlayerHUD>(PC->GetHUD())) {
      HUD->HidePauseMenu();
    }
  }
}

void UPauseWidget::OnMainMenuClicked() {
  // Hide pause before transitioning to prevent input lock issues
  OnResumeClicked();
  UGameplayStatics::OpenLevel(this, FName("MainMenuMap"));
}

void UPauseWidget::OnSettingsClicked() {
  // TBD: Toggle visibility of a settings panel/widget
  UE_LOG(LogTemp, Warning, TEXT("Settings clicked - Placeholder"));
}

void UPauseWidget::OnDevSettingsClicked() {
  if (APlayerController *PC = GetOwningPlayer()) {
    if (ADragonSlayerHUD *HUD = Cast<ADragonSlayerHUD>(PC->GetHUD())) {
      HUD->ShowDeveloperSettingsWidget();
    }
  }
}

void UPauseWidget::OnExitClicked() {
  UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit,
                                 false);
}
