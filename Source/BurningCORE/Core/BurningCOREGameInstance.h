#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BurningCOREGameInstance.generated.h"

class UBurningCORESaveGame;

/**
 * UBurningCOREGameInstance
 * Manages save/load persistence and global state across levels.
 */
UCLASS()
class BURNINGCORE_API UBurningCOREGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	/** Saves progress to the active slot */
	UFUNCTION(BlueprintCallable, Category="Save")
	bool SaveProgress();
    
	/** Loads progress from a slot index */
	UFUNCTION(BlueprintCallable, Category="Save")
	bool LoadProgress(int32 SlotIndex);

	UPROPERTY(BlueprintReadOnly, Category="Save")
	TObjectPtr<UBurningCORESaveGame> CurrentSave;

	UPROPERTY(EditDefaultsOnly, Category="Save")
	FString SaveSlotName = TEXT("SaveSlot_1");
};
