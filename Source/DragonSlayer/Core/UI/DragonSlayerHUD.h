#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DragonSlayerHUD.generated.h"

class UPauseWidget;
class UDeveloperSettingsWidget;

UCLASS()
class DRAGONSLAYER_API ADragonSlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	ADragonSlayerHUD();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HidePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleDeveloperSettingsWidget();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowDeveloperSettingsWidget();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideDeveloperSettingsWidget();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPauseWidget> PauseWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UDeveloperSettingsWidget> DeveloperSettingsWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UPauseWidget> PauseWidgetInstance;

	UPROPERTY()
	TObjectPtr<UDeveloperSettingsWidget> DeveloperSettingsWidgetInstance;
};
