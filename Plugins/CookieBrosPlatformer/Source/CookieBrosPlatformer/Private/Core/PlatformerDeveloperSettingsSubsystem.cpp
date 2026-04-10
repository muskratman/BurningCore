#include "Core/PlatformerDeveloperSettingsSubsystem.h"

#include "Core/SaveGame/SaveDeveloperSettings.h"
#include "Core/SaveGame/SaveDeveloperSettingsSlotIndex.h"
#include "Kismet/GameplayStatics.h"

namespace
{
const FString DeveloperSettingsSlotIndexSaveSlotName = TEXT("SaveDeveloperSettings_Index");
const FString DeveloperSettingsLegacySaveSlotName = TEXT("SaveDeveloperSettings_1");
const FString DeveloperSettingsSlotPrefix = TEXT("SaveDeveloperSettings_Slot_");
const FString ImportedLegacyDeveloperSettingsDisplayName = TEXT("Imported Legacy Settings");
constexpr int32 DeveloperSettingsSaveUserIndex = 0;
}

void UPlatformerDeveloperSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadSlotIndex();
	MigrateLegacyDeveloperSettingsIfNeeded();
}

void UPlatformerDeveloperSettingsSubsystem::Deinitialize()
{
	SlotIndexSave = nullptr;
	Super::Deinitialize();
}

const TArray<FPlatformerDeveloperSettingsSlotDescriptor>& UPlatformerDeveloperSettingsSubsystem::GetAvailableSlots() const
{
	static const TArray<FPlatformerDeveloperSettingsSlotDescriptor> EmptySlots;
	return SlotIndexSave ? SlotIndexSave->Slots : EmptySlots;
}

bool UPlatformerDeveloperSettingsSubsystem::HasCurrentSlot() const
{
	if (!SlotIndexSave || !SlotIndexSave->CurrentSlotId.IsValid())
	{
		return false;
	}

	return FindSlotDescriptorById(SlotIndexSave->CurrentSlotId) != nullptr;
}

FString UPlatformerDeveloperSettingsSubsystem::GetCurrentSlotDisplayName() const
{
	FPlatformerDeveloperSettingsSlotDescriptor CurrentSlot;
	return TryGetCurrentSlot(CurrentSlot) ? CurrentSlot.DisplayName : FString();
}

bool UPlatformerDeveloperSettingsSubsystem::TryGetCurrentSlot(FPlatformerDeveloperSettingsSlotDescriptor& OutSlot) const
{
	if (!HasCurrentSlot())
	{
		return false;
	}

	if (const FPlatformerDeveloperSettingsSlotDescriptor* CurrentSlot = FindSlotDescriptorById(SlotIndexSave->CurrentSlotId))
	{
		OutSlot = *CurrentSlot;
		return true;
	}

	return false;
}

bool UPlatformerDeveloperSettingsSubsystem::TryLoadCurrentSnapshot(FPlatformerDeveloperSettingsSnapshot& OutSnapshot) const
{
	if (!HasCurrentSlot())
	{
		return false;
	}

	if (USaveDeveloperSettings* LoadedSave = LoadDeveloperSavePayload(SlotIndexSave->CurrentSlotId))
	{
		OutSnapshot = LoadedSave->ResolveSnapshot();
		return true;
	}

	return false;
}

bool UPlatformerDeveloperSettingsSubsystem::SaveCurrent(const FPlatformerDeveloperSettingsSnapshot& Snapshot)
{
	if (!HasCurrentSlot())
	{
		return false;
	}

	FPlatformerDeveloperSettingsSlotDescriptor* CurrentSlot = FindMutableSlotDescriptorById(SlotIndexSave->CurrentSlotId);
	if (!CurrentSlot)
	{
		return false;
	}

	if (!WriteDeveloperSavePayload(CurrentSlot->SlotId, Snapshot))
	{
		return false;
	}

	CurrentSlot->UpdatedAtUtc = FDateTime::UtcNow();
	return SaveSlotIndex();
}

bool UPlatformerDeveloperSettingsSubsystem::SaveAs(
	const FString& DesiredDisplayName,
	const FPlatformerDeveloperSettingsSnapshot& Snapshot,
	FPlatformerDeveloperSettingsSlotDescriptor& OutSlot)
{
	const FString SanitizedDisplayName = SanitizeDisplayName(DesiredDisplayName);
	if (SanitizedDisplayName.IsEmpty())
	{
		return false;
	}

	EnsureSlotIndexObject();

	FPlatformerDeveloperSettingsSlotDescriptor* ExistingSlot = FindMutableSlotDescriptorByDisplayName(SanitizedDisplayName);
	const FDateTime Timestamp = FDateTime::UtcNow();
	FGuid SlotId = ExistingSlot ? ExistingSlot->SlotId : FGuid::NewGuid();

	if (!WriteDeveloperSavePayload(SlotId, Snapshot))
	{
		return false;
	}

	if (ExistingSlot)
	{
		ExistingSlot->DisplayName = SanitizedDisplayName;
		ExistingSlot->UpdatedAtUtc = Timestamp;
		OutSlot = *ExistingSlot;
	}
	else
	{
		FPlatformerDeveloperSettingsSlotDescriptor NewSlot;
		NewSlot.SlotId = SlotId;
		NewSlot.DisplayName = SanitizedDisplayName;
		NewSlot.CreatedAtUtc = Timestamp;
		NewSlot.UpdatedAtUtc = Timestamp;
		SlotIndexSave->Slots.Add(NewSlot);
		OutSlot = NewSlot;
	}

	SlotIndexSave->CurrentSlotId = SlotId;
	return SaveSlotIndex();
}

bool UPlatformerDeveloperSettingsSubsystem::LoadSlot(const FGuid& SlotId, FPlatformerDeveloperSettingsSnapshot& OutSnapshot)
{
	if (!SlotId.IsValid() || !FindSlotDescriptorById(SlotId))
	{
		return false;
	}

	if (USaveDeveloperSettings* LoadedSave = LoadDeveloperSavePayload(SlotId))
	{
		OutSnapshot = LoadedSave->ResolveSnapshot();
		SlotIndexSave->CurrentSlotId = SlotId;
		return SaveSlotIndex();
	}

	return false;
}

bool UPlatformerDeveloperSettingsSubsystem::DeleteSlot(const FGuid& SlotId)
{
	if (!SlotId.IsValid())
	{
		return false;
	}

	FPlatformerDeveloperSettingsSlotDescriptor* SlotToDelete = FindMutableSlotDescriptorById(SlotId);
	if (!SlotToDelete)
	{
		return false;
	}

	const FString SlotName = BuildDeveloperSettingsSlotName(SlotId);
	const bool bHasStoredPayload = UGameplayStatics::DoesSaveGameExist(SlotName, DeveloperSettingsSaveUserIndex);
	if (bHasStoredPayload && !UGameplayStatics::DeleteGameInSlot(SlotName, DeveloperSettingsSaveUserIndex))
	{
		return false;
	}

	SlotIndexSave->Slots.RemoveAll([&SlotId](const FPlatformerDeveloperSettingsSlotDescriptor& Slot)
	{
		return Slot.SlotId == SlotId;
	});

	if (SlotIndexSave->CurrentSlotId == SlotId)
	{
		SlotIndexSave->CurrentSlotId.Invalidate();
	}

	return SaveSlotIndex();
}

void UPlatformerDeveloperSettingsSubsystem::EnsureSlotIndexObject()
{
	if (!SlotIndexSave)
	{
		SlotIndexSave = Cast<USaveDeveloperSettingsSlotIndex>(
			UGameplayStatics::CreateSaveGameObject(USaveDeveloperSettingsSlotIndex::StaticClass()));
	}
}

bool UPlatformerDeveloperSettingsSubsystem::LoadSlotIndex()
{
	if (UGameplayStatics::DoesSaveGameExist(DeveloperSettingsSlotIndexSaveSlotName, DeveloperSettingsSaveUserIndex))
	{
		SlotIndexSave = Cast<USaveDeveloperSettingsSlotIndex>(
			UGameplayStatics::LoadGameFromSlot(DeveloperSettingsSlotIndexSaveSlotName, DeveloperSettingsSaveUserIndex));
	}

	EnsureSlotIndexObject();

	if (!SlotIndexSave)
	{
		return false;
	}

	if (SlotIndexSave->CurrentSlotId.IsValid() && !FindSlotDescriptorById(SlotIndexSave->CurrentSlotId))
	{
		SlotIndexSave->CurrentSlotId.Invalidate();
	}

	return true;
}

bool UPlatformerDeveloperSettingsSubsystem::SaveSlotIndex()
{
	EnsureSlotIndexObject();
	return SlotIndexSave != nullptr
		&& UGameplayStatics::SaveGameToSlot(SlotIndexSave, DeveloperSettingsSlotIndexSaveSlotName, DeveloperSettingsSaveUserIndex);
}

void UPlatformerDeveloperSettingsSubsystem::MigrateLegacyDeveloperSettingsIfNeeded()
{
	if (!SlotIndexSave || !SlotIndexSave->Slots.IsEmpty() || SlotIndexSave->CurrentSlotId.IsValid())
	{
		return;
	}

	if (!UGameplayStatics::DoesSaveGameExist(DeveloperSettingsLegacySaveSlotName, DeveloperSettingsSaveUserIndex))
	{
		return;
	}

	USaveDeveloperSettings* LegacySave = Cast<USaveDeveloperSettings>(
		UGameplayStatics::LoadGameFromSlot(DeveloperSettingsLegacySaveSlotName, DeveloperSettingsSaveUserIndex));
	if (!LegacySave)
	{
		return;
	}

	FPlatformerDeveloperSettingsSlotDescriptor ImportedSlot;
	if (SaveAs(ImportedLegacyDeveloperSettingsDisplayName, LegacySave->ResolveSnapshot(), ImportedSlot))
	{
		SlotIndexSave->CurrentSlotId = ImportedSlot.SlotId;
		SaveSlotIndex();
	}
}

FString UPlatformerDeveloperSettingsSubsystem::SanitizeDisplayName(const FString& InDisplayName) const
{
	return InDisplayName.TrimStartAndEnd();
}

FString UPlatformerDeveloperSettingsSubsystem::BuildDeveloperSettingsSlotName(const FGuid& SlotId) const
{
	return DeveloperSettingsSlotPrefix + SlotId.ToString(EGuidFormats::DigitsWithHyphens);
}

USaveDeveloperSettings* UPlatformerDeveloperSettingsSubsystem::CreateDeveloperSavePayload() const
{
	return Cast<USaveDeveloperSettings>(UGameplayStatics::CreateSaveGameObject(USaveDeveloperSettings::StaticClass()));
}

USaveDeveloperSettings* UPlatformerDeveloperSettingsSubsystem::LoadDeveloperSavePayload(const FGuid& SlotId) const
{
	if (!SlotId.IsValid())
	{
		return nullptr;
	}

	const FString SlotName = BuildDeveloperSettingsSlotName(SlotId);
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, DeveloperSettingsSaveUserIndex))
	{
		return nullptr;
	}

	return Cast<USaveDeveloperSettings>(UGameplayStatics::LoadGameFromSlot(SlotName, DeveloperSettingsSaveUserIndex));
}

bool UPlatformerDeveloperSettingsSubsystem::WriteDeveloperSavePayload(
	const FGuid& SlotId,
	const FPlatformerDeveloperSettingsSnapshot& Snapshot)
{
	USaveDeveloperSettings* SavePayload = CreateDeveloperSavePayload();
	if (!SavePayload || !SlotId.IsValid())
	{
		return false;
	}

	SavePayload->SlotId = SlotId;
	SavePayload->SetSnapshot(Snapshot);
	return UGameplayStatics::SaveGameToSlot(
		SavePayload,
		BuildDeveloperSettingsSlotName(SlotId),
		DeveloperSettingsSaveUserIndex);
}

const FPlatformerDeveloperSettingsSlotDescriptor* UPlatformerDeveloperSettingsSubsystem::FindSlotDescriptorById(const FGuid& SlotId) const
{
	if (!SlotIndexSave || !SlotId.IsValid())
	{
		return nullptr;
	}

	return SlotIndexSave->Slots.FindByPredicate([&SlotId](const FPlatformerDeveloperSettingsSlotDescriptor& Slot)
	{
		return Slot.SlotId == SlotId;
	});
}

FPlatformerDeveloperSettingsSlotDescriptor* UPlatformerDeveloperSettingsSubsystem::FindMutableSlotDescriptorById(const FGuid& SlotId)
{
	if (!SlotIndexSave || !SlotId.IsValid())
	{
		return nullptr;
	}

	return SlotIndexSave->Slots.FindByPredicate([&SlotId](const FPlatformerDeveloperSettingsSlotDescriptor& Slot)
	{
		return Slot.SlotId == SlotId;
	});
}

const FPlatformerDeveloperSettingsSlotDescriptor* UPlatformerDeveloperSettingsSubsystem::FindSlotDescriptorByDisplayName(const FString& DisplayName) const
{
	if (!SlotIndexSave)
	{
		return nullptr;
	}

	const FString SanitizedDisplayName = SanitizeDisplayName(DisplayName);
	if (SanitizedDisplayName.IsEmpty())
	{
		return nullptr;
	}

	return SlotIndexSave->Slots.FindByPredicate([&SanitizedDisplayName](const FPlatformerDeveloperSettingsSlotDescriptor& Slot)
	{
		return Slot.DisplayName.Equals(SanitizedDisplayName, ESearchCase::IgnoreCase);
	});
}

FPlatformerDeveloperSettingsSlotDescriptor* UPlatformerDeveloperSettingsSubsystem::FindMutableSlotDescriptorByDisplayName(const FString& DisplayName)
{
	if (!SlotIndexSave)
	{
		return nullptr;
	}

	const FString SanitizedDisplayName = SanitizeDisplayName(DisplayName);
	if (SanitizedDisplayName.IsEmpty())
	{
		return nullptr;
	}

	return SlotIndexSave->Slots.FindByPredicate([&SanitizedDisplayName](const FPlatformerDeveloperSettingsSlotDescriptor& Slot)
	{
		return Slot.DisplayName.Equals(SanitizedDisplayName, ESearchCase::IgnoreCase);
	});
}
