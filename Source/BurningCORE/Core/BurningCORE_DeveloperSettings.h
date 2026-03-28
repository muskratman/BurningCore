#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "BurningCORE_DeveloperSettings.generated.h"

/**
 * Developer settings for gameplay tweaking.
 * Can be hot-reloaded or modified in-game by the Dev UI.
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="BurningCORE Dev Settings"))
class BURNINGCORE_API UBurningCORE_DeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UBurningCORE_DeveloperSettings();

	UPROPERTY(Config, EditAnywhere, Category="Movement", meta=(ClampMin="0.0"))
	float DevGravityScale;

	UPROPERTY(Config, EditAnywhere, Category="Movement", meta=(ClampMin="0.0"))
	float DevJumpHeight;

	UPROPERTY(Config, EditAnywhere, Category="Movement", meta=(ClampMin="0.0", ClampMax="1.0"))
	float DevAirControl;

	// Note: GAS Cooldowns (Dash, BaseShot delays) postponed per user request.
};
