#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UComboBoxString;
class UButton;

/**
 * Widget for the Main Menu. Displays available levels and allows the player to start the game.
 */
UCLASS(Abstract)
class BURNINGCORE_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> LevelComboBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartGameButton;

	UFUNCTION()
	void OnStartGameClicked();

private:
	void PopulateLevels();
};
