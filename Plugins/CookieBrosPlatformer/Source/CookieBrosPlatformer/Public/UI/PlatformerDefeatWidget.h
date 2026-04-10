#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "PlatformerDefeatWidget.generated.h"

class UButton;

UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API UPlatformerDefeatWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Btn_Restart;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Btn_MainMenu;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Btn_Exit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Defeat")
	FName MainMenuLevelName = TEXT("MainMenuMap");

private:
	void CloseDefeatMenu();

	UFUNCTION()
	void OnRestartClicked();

	UFUNCTION()
	void OnMainMenuClicked();

	UFUNCTION()
	void OnExitClicked();
};
