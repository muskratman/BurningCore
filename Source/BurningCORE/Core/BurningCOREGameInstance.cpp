#include "Core/BurningCOREGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/BurningCORESaveGame.h"

void UBurningCOREGameInstance::Init()
{
	Super::Init();
}

bool UBurningCOREGameInstance::SaveProgress()
{
	if (!CurrentSave) return false;
	
	// Delegate saving logic to ProgressionSubsystem before actual save
	return UGameplayStatics::SaveGameToSlot(CurrentSave.Get(), SaveSlotName, 0);
}

bool UBurningCOREGameInstance::LoadProgress(int32 SlotIndex)
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		CurrentSave = Cast<UBurningCORESaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
		return CurrentSave != nullptr;
	}
	return false;
}
