// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlatformerPlayerControllerBase.generated.h"

class UInputAction;
class UInputMappingContext;
class UUserWidget;
struct FInputActionValue;

/**
 * Generic platformer player controller shell with Enhanced Input setup, touch controls,
 * optional legacy respawn handling, and overridable pause/defeat hooks.
 */
UCLASS(Config="Game", Abstract)
class COOKIEBROSPLATFORMER_API APlatformerPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void HandleControlledCharacterDeath();

protected:
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<TObjectPtr<UInputMappingContext>> MobileExcludedMappingContexts;

	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	UPROPERTY(EditAnywhere, Config, Category="Input|Touch Controls")
	bool bForceTouchControls = false;

	UPROPERTY(EditAnywhere, Category="Respawn")
	TSubclassOf<APawn> RespawnPawnClass;

	UPROPERTY(EditAnywhere, Category="Respawn")
	bool bAutoRespawnOnPawnDestroyed = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Input Mappings")
	TObjectPtr<UInputAction> TogglePauseAction;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);

	bool ShouldUseTouchControls() const;
	virtual bool TryGetRespawnTransform(FTransform& OutSpawnTransform) const;
	virtual APawn* SpawnRespawnPawn(const FTransform& SpawnTransform);
	virtual void HandlePauseRequested();

private:
	void OnTogglePause(const FInputActionValue& Value);
};
