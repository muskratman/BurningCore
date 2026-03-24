#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * IInteractable
 * Interface for interactive objects (NPCs, shrines, doors).
 */
class BURNINGCORE_API IInteractable
{
	GENERATED_BODY()

public:
	/** Executes interaction logic */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
	void Interact(AActor* Interactor);

	/** Checks if interaction is currently valid */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
	bool CanInteract(AActor* Interactor) const;

	/** UI prompt text */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
	FText GetInteractionPrompt() const;
};
