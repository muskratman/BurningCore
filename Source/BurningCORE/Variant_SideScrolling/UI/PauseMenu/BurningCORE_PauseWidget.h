#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BurningCORE_PauseWidget.generated.h"

class UButton;

UCLASS(Abstract)
class BURNINGCORE_API UBurningCORE_PauseWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Resume;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_MainMenu;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Settings;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_DevSettings;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Exit;

private:
	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnMainMenuClicked();

	UFUNCTION()
	void OnSettingsClicked();

	UFUNCTION()
	void OnDevSettingsClicked();

	UFUNCTION()
	void OnExitClicked();
};
