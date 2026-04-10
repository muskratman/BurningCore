#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Traversal/PlatformerTraversalTypes.h"
#include "PlatformerTraversalComponent.generated.h"

class UPlatformerTraversalConfigDataAsset;
class UPlatformerTraversalMovementComponent;

/**
 * Owns editor-facing traversal config and forwards it into the traversal-aware movement component.
 * Movement execution stays in CharacterMovementComponent while tuning/integration live in a separate component.
 */
UCLASS(ClassGroup=(Traversal), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class COOKIEBROSPLATFORMER_API UPlatformerTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlatformerTraversalComponent();

	virtual void OnRegister() override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Traversal")
	void SetTraversalEnabled(bool bInTraversalEnabled);

	UFUNCTION(BlueprintPure, Category="Traversal")
	bool IsTraversalEnabled() const { return bTraversalEnabled; }

	UFUNCTION(BlueprintCallable, Category="Traversal")
	void SetTraversalConfig(UPlatformerTraversalConfigDataAsset* InTraversalConfig);

	UFUNCTION(BlueprintPure, Category="Traversal")
	UPlatformerTraversalConfigDataAsset* GetTraversalConfig() const { return TraversalConfig; }

	UFUNCTION(BlueprintPure, Category="Traversal")
	UPlatformerTraversalMovementComponent* GetTraversalMovementComponent() const;

	UFUNCTION(BlueprintCallable, Category="Traversal")
	void ApplyTraversalSettings();

	void SetDeveloperTraversalSettingsOverride(
		const FPlatformerLedgeTraversalSettings& InLedgeSettings,
		const FPlatformerSlideDashSettings& InSlideDashSettings,
		const FPlatformerWallTraversalSettings& InWallSettings);
	void ClearDeveloperTraversalSettingsOverride();
	bool HasDeveloperTraversalSettingsOverride() const { return bHasDeveloperTraversalSettingsOverride; }
	FPlatformerLedgeTraversalSettings GetResolvedLedgeSettings() const;
	FPlatformerSlideDashSettings GetResolvedSlideDashSettings() const;
	FPlatformerWallTraversalSettings GetResolvedWallSettings() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Traversal")
	bool bTraversalEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Traversal")
	TObjectPtr<UPlatformerTraversalConfigDataAsset> TraversalConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Traversal|Fallback")
	FPlatformerLedgeTraversalSettings DefaultLedgeSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Traversal|Fallback")
	FPlatformerSlideDashSettings DefaultSlideDashSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Traversal|Fallback")
	FPlatformerWallTraversalSettings DefaultWallSettings;

	UPROPERTY(Transient)
	bool bHasDeveloperTraversalSettingsOverride = false;

	UPROPERTY(Transient)
	FPlatformerLedgeTraversalSettings DeveloperLedgeSettingsOverride;

	UPROPERTY(Transient)
	FPlatformerSlideDashSettings DeveloperSlideDashSettingsOverride;

	UPROPERTY(Transient)
	FPlatformerWallTraversalSettings DeveloperWallSettingsOverride;
};
