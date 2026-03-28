#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BurningCORE_DevSettingsWidget.generated.h"

UCLASS(Abstract)
class BURNINGCORE_API UBurningCORE_DevSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	
	// Base class for DevSettings. Sliders and bindings to UBurningCORE_DeveloperSettings will be added here
};
