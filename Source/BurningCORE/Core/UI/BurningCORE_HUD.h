#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BurningCORE_HUD.generated.h"

class UBurningCORE_PauseWidget;

UCLASS()
class BURNINGCORE_API ABurningCORE_HUD : public AHUD
{
	GENERATED_BODY()

public:
	ABurningCORE_HUD();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HidePauseMenu();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UBurningCORE_PauseWidget> PauseWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UBurningCORE_PauseWidget> PauseWidgetInstance;
};
