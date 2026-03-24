#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UComboBoxString;
class UButton;
class UTextBlock;

/**
 * Widget for the Main Menu. Programmatically builds UI with level selection and start button.
 */
UCLASS()
class BURNINGCORE_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnStartGameClicked();

private:
	void PopulateLevels();

	UPROPERTY()
	TObjectPtr<UComboBoxString> LevelComboBox;

	UPROPERTY()
	TObjectPtr<UButton> StartGameButton;

	UPROPERTY()
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY()
	TObjectPtr<UTextBlock> StartButtonText;
};
