#pragma once

#include "CoreMinimal.h"
#include "Developer/DeveloperPlatformerSettingsTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlatformerDeveloperSettingsSubsystem.generated.h"

class USaveDeveloperSettings;
class USaveDeveloperSettingsSlotIndex;

UCLASS()
class COOKIEBROSPLATFORMER_API UPlatformerDeveloperSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	const TArray<FPlatformerDeveloperSettingsSlotDescriptor>& GetAvailableSlots() const;
	bool HasCurrentSlot() const;
	FString GetCurrentSlotDisplayName() const;
	bool TryGetCurrentSlot(FPlatformerDeveloperSettingsSlotDescriptor& OutSlot) const;
	bool TryLoadCurrentSnapshot(FPlatformerDeveloperSettingsSnapshot& OutSnapshot) const;
	bool SaveCurrent(const FPlatformerDeveloperSettingsSnapshot& Snapshot);
	bool SaveAs(const FString& DesiredDisplayName, const FPlatformerDeveloperSettingsSnapshot& Snapshot, FPlatformerDeveloperSettingsSlotDescriptor& OutSlot);
	bool LoadSlot(const FGuid& SlotId, FPlatformerDeveloperSettingsSnapshot& OutSnapshot);
	bool DeleteSlot(const FGuid& SlotId);

private:
	void EnsureSlotIndexObject();
	bool LoadSlotIndex();
	bool SaveSlotIndex();
	void MigrateLegacyDeveloperSettingsIfNeeded();
	FString SanitizeDisplayName(const FString& InDisplayName) const;
	FString BuildDeveloperSettingsSlotName(const FGuid& SlotId) const;
	USaveDeveloperSettings* CreateDeveloperSavePayload() const;
	USaveDeveloperSettings* LoadDeveloperSavePayload(const FGuid& SlotId) const;
	bool WriteDeveloperSavePayload(const FGuid& SlotId, const FPlatformerDeveloperSettingsSnapshot& Snapshot);
	const FPlatformerDeveloperSettingsSlotDescriptor* FindSlotDescriptorById(const FGuid& SlotId) const;
	FPlatformerDeveloperSettingsSlotDescriptor* FindMutableSlotDescriptorById(const FGuid& SlotId);
	const FPlatformerDeveloperSettingsSlotDescriptor* FindSlotDescriptorByDisplayName(const FString& DisplayName) const;
	FPlatformerDeveloperSettingsSlotDescriptor* FindMutableSlotDescriptorByDisplayName(const FString& DisplayName);

	UPROPERTY(Transient)
	TObjectPtr<USaveDeveloperSettingsSlotIndex> SlotIndexSave;
};
