#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DragonSlayerHUD.generated.h"

class UPauseWidget;
class UDeveloperSettingsWidget;
class UPlatformerDefeatWidget;

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

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowDefeatWidget();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideDefeatWidget();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPauseWidget> PauseWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UDeveloperSettingsWidget> DeveloperSettingsWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPlatformerDefeatWidget> DefeatWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UPauseWidget> PauseWidgetInstance;

	UPROPERTY()
	TObjectPtr<UDeveloperSettingsWidget> DeveloperSettingsWidgetInstance;

	UPROPERTY()
	TObjectPtr<UPlatformerDefeatWidget> DefeatWidgetInstance;
};
