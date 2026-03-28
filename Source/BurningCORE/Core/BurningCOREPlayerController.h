#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BurningCOREPlayerController.generated.h"

class UInputMappingContext;
class UCameraRailAsset;

/**
 * ABurningCOREPlayerController
 * Handles input mapping and camera mode switching.
 */
UCLASS()
class BURNINGCORE_API ABurningCOREPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABurningCOREPlayerController();

	virtual void SetupInputComponent() override;

	/** Transitions the camera to follow a specific target/rail */
	UFUNCTION(BlueprintCallable, Category="Camera")
	void SwitchToSideViewCamera(AActor* CameraTarget);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> PlatformerMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	class UInputAction* TogglePauseAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	int32 MappingContextPriority = 0;

private:
	void OnTogglePause(const struct FInputActionValue& Value);
};
