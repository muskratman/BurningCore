#include "Traversal/PlatformerTraversalComponent.h"

#include "GameFramework/Character.h"
#include "Traversal/PlatformerTraversalConfigDataAsset.h"
#include "Traversal/PlatformerTraversalMovementComponent.h"

namespace
{
FPlatformerLedgeTraversalSettings MakeDisabledLedgeSettings()
{
	FPlatformerLedgeTraversalSettings Settings;
	Settings.bEnabled = false;
	return Settings;
}

FPlatformerDashSettings MakeDisabledDashSettings()
{
	FPlatformerDashSettings Settings;
	Settings.bEnabled = false;
	return Settings;
}

FPlatformerWallTraversalSettings MakeDisabledWallSettings()
{
	FPlatformerWallTraversalSettings Settings;
	Settings.bEnabled = false;
	return Settings;
}

FPlatformerLedgeTraversalSettings PreserveLedgeAvailability(
	const FPlatformerLedgeTraversalSettings& Settings,
	const UPlatformerTraversalConfigDataAsset* TraversalConfig)
{
	FPlatformerLedgeTraversalSettings ResolvedSettings = Settings;
	ResolvedSettings.bEnabled = TraversalConfig ? TraversalConfig->LedgeSettings.bEnabled : false;
	return ResolvedSettings;
}

FPlatformerDashSettings PreserveDashAvailability(
	const FPlatformerDashSettings& Settings,
	const UPlatformerTraversalConfigDataAsset* TraversalConfig)
{
	FPlatformerDashSettings ResolvedSettings = Settings;
	ResolvedSettings.bEnabled = TraversalConfig ? TraversalConfig->DashSettings.bEnabled : false;
	return ResolvedSettings;
}

FPlatformerWallTraversalSettings PreserveWallAvailability(
	const FPlatformerWallTraversalSettings& Settings,
	const UPlatformerTraversalConfigDataAsset* TraversalConfig)
{
	FPlatformerWallTraversalSettings ResolvedSettings = Settings;
	ResolvedSettings.bEnabled = TraversalConfig ? TraversalConfig->WallSettings.bEnabled : false;
	return ResolvedSettings;
}
}

UPlatformerTraversalComponent::UPlatformerTraversalComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlatformerTraversalComponent::OnRegister()
{
	Super::OnRegister();
	ApplyTraversalSettings();
}

void UPlatformerTraversalComponent::BeginPlay()
{
	Super::BeginPlay();
	ApplyTraversalSettings();
}

void UPlatformerTraversalComponent::SetTraversalEnabled(bool bInTraversalEnabled)
{
	bTraversalEnabled = bInTraversalEnabled;
	ApplyTraversalSettings();
}

void UPlatformerTraversalComponent::SetTraversalConfig(UPlatformerTraversalConfigDataAsset* InTraversalConfig)
{
	TraversalConfig = InTraversalConfig;
	ApplyTraversalSettings();
}

void UPlatformerTraversalComponent::SetDeveloperTraversalSettingsOverride(
	const FPlatformerLedgeTraversalSettings& InLedgeSettings,
	const FPlatformerDashSettings& InDashSettings,
	const FPlatformerWallTraversalSettings& InWallSettings)
{
	bHasDeveloperTraversalSettingsOverride = true;
	DeveloperLedgeSettingsOverride = InLedgeSettings;
	DeveloperDashSettingsOverride = InDashSettings;
	DeveloperWallSettingsOverride = InWallSettings;
	ApplyTraversalSettings();
}

void UPlatformerTraversalComponent::ClearDeveloperTraversalSettingsOverride()
{
	bHasDeveloperTraversalSettingsOverride = false;
	ApplyTraversalSettings();
}

UPlatformerTraversalMovementComponent* UPlatformerTraversalComponent::GetTraversalMovementComponent() const
{
	const ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
	return CharacterOwner ? Cast<UPlatformerTraversalMovementComponent>(CharacterOwner->GetCharacterMovement()) : nullptr;
}

void UPlatformerTraversalComponent::ApplyTraversalSettings()
{
	if (UPlatformerTraversalMovementComponent* TraversalMovementComponent = GetTraversalMovementComponent())
	{
		if (!TraversalConfig)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: TraversalConfig is not assigned on %s. Traversal is disabled until a UPlatformerTraversalConfigDataAsset is configured."),
				*GetNameSafe(this),
				*GetNameSafe(GetOwner()));
			TraversalMovementComponent->SetTraversalConfig(nullptr);
			TraversalMovementComponent->ClearDeveloperTraversalSettingsOverride();
			TraversalMovementComponent->SetTraversalEnabled(false);
			return;
		}

		TraversalMovementComponent->SetTraversalConfig(TraversalConfig);

		if (bHasDeveloperTraversalSettingsOverride)
		{
			TraversalMovementComponent->SetDeveloperTraversalSettingsOverride(
				GetResolvedLedgeSettings(),
				GetResolvedDashSettings(),
				GetResolvedWallSettings());
		}
		else
		{
			TraversalMovementComponent->ClearDeveloperTraversalSettingsOverride();
		}

		TraversalMovementComponent->SetTraversalEnabled(bTraversalEnabled);
	}
}

FPlatformerLedgeTraversalSettings UPlatformerTraversalComponent::GetResolvedLedgeSettings() const
{
	if (bHasDeveloperTraversalSettingsOverride)
	{
		return PreserveLedgeAvailability(DeveloperLedgeSettingsOverride, TraversalConfig);
	}

	return TraversalConfig ? TraversalConfig->LedgeSettings : MakeDisabledLedgeSettings();
}

FPlatformerDashSettings UPlatformerTraversalComponent::GetResolvedDashSettings() const
{
	if (bHasDeveloperTraversalSettingsOverride)
	{
		return PreserveDashAvailability(DeveloperDashSettingsOverride, TraversalConfig);
	}

	return TraversalConfig ? TraversalConfig->DashSettings : MakeDisabledDashSettings();
}

FPlatformerWallTraversalSettings UPlatformerTraversalComponent::GetResolvedWallSettings() const
{
	if (bHasDeveloperTraversalSettingsOverride)
	{
		return PreserveWallAvailability(DeveloperWallSettingsOverride, TraversalConfig);
	}

	return TraversalConfig ? TraversalConfig->WallSettings : MakeDisabledWallSettings();
}
