// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlatformerPlayerController.h"
#include "Core/UI/DragonSlayerHUD.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Platformer/Character/PlayableDragonCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"

APlatformerPlayerController::APlatformerPlayerController()
{
	static ConstructorHelpers::FObjectFinder<UInputAction> TogglePauseActionFinder(
		TEXT("/Game/Blueprints/Input/Actions/IA_TogglePause.IA_TogglePause"));
	if (TogglePauseActionFinder.Succeeded())
	{
		TogglePauseAction = TogglePauseActionFinder.Object;
		TogglePauseAction->bTriggerWhenPaused = true;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> ToggleDeveloperActionFinder(
		TEXT("/Game/Blueprints/Input/Actions/IA_ToggleDeveloper.IA_ToggleDeveloper"));
	if (ToggleDeveloperActionFinder.Succeeded())
	{
		ToggleDeveloperAction = ToggleDeveloperActionFinder.Object;
		ToggleDeveloperAction->bTriggerWhenPaused = true;
	}
}

void APlatformerPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ToggleDeveloperAction)
		{
			EnhancedInputComponent->BindAction(
				ToggleDeveloperAction,
				ETriggerEvent::Started,
				this,
				&APlatformerPlayerController::HandleDeveloperSettingsToggleRequested);
		}
	}
}

APawn* APlatformerPlayerController::SpawnRespawnPawn(const FTransform& SpawnTransform)
{
	if (CharacterClass && GetWorld())
	{
		return GetWorld()->SpawnActor<APlayableDragonCharacter>(CharacterClass, SpawnTransform);
	}

	return Super::SpawnRespawnPawn(SpawnTransform);
}

void APlatformerPlayerController::HandlePauseRequested()
{
	if (ADragonSlayerHUD* BurningHUD = Cast<ADragonSlayerHUD>(GetHUD()))
	{
		BurningHUD->TogglePauseMenu();
	}
}

void APlatformerPlayerController::HandleDeveloperSettingsToggleRequested()
{
	if (ADragonSlayerHUD* BurningHUD = Cast<ADragonSlayerHUD>(GetHUD()))
	{
		BurningHUD->ToggleDeveloperSettingsWidget();
	}
}
