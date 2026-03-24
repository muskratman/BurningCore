#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MainMenuHUD.generated.h"

class UMainMenuWidget;

/**
 * HUD for the Main Menu. Creates and manages the MainMenuWidget.
 */
UCLASS()
class BURNINGCORE_API AMainMenuHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	UPROPERTY()
	TObjectPtr<UMainMenuWidget> MainMenuWidget;
};
