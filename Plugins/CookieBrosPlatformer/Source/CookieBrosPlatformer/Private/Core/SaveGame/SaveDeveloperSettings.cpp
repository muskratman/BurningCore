#include "Core/SaveGame/SaveDeveloperSettings.h"

namespace
{
constexpr int32 DeveloperSettingsSnapshotDataVersion = 1;
}

void USaveDeveloperSettings::SetSnapshot(const FPlatformerDeveloperSettingsSnapshot& InSnapshot)
{
	DataVersion = DeveloperSettingsSnapshotDataVersion;
	Snapshot = InSnapshot;
	DeveloperCharacterSettings = InSnapshot.CharacterSettings;
	DeveloperCameraManagerSettings = InSnapshot.CameraManagerSettings;
	bHasSavedDeveloperCombatSettings = InSnapshot.bHasSavedCombatSettings;
	bAutoRestartLevel = InSnapshot.bAutoRestartLevel;
}

FPlatformerDeveloperSettingsSnapshot USaveDeveloperSettings::ResolveSnapshot() const
{
	if (UsesSnapshotData())
	{
		return Snapshot;
	}

	FPlatformerDeveloperSettingsSnapshot LegacySnapshot;
	LegacySnapshot.CharacterSettings = DeveloperCharacterSettings;
	LegacySnapshot.CameraManagerSettings = DeveloperCameraManagerSettings;
	LegacySnapshot.bHasSavedCombatSettings = bHasSavedDeveloperCombatSettings;
	LegacySnapshot.bAutoRestartLevel = bAutoRestartLevel;
	return LegacySnapshot;
}

bool USaveDeveloperSettings::UsesSnapshotData() const
{
	return DataVersion >= DeveloperSettingsSnapshotDataVersion;
}
