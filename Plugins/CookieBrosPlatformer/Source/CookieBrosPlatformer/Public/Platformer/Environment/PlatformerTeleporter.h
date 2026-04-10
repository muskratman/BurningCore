#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlatformerTeleporter.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class UTexture2D;

/**
 * Teleporter that routes point A to another teleporter's actor transform.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerTeleporter : public AActor
{
	GENERATED_BODY()

public:
	APlatformerTeleporter();

	void RegisterArrival(AActor* Actor);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> TriggerVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> TeleporterMesh;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Teleporter")
	TObjectPtr<APlatformerTeleporter> ExitTeleporter;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Teleporter")
	FString TeleporterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Teleporter")
	FVector TriggerExtent = FVector(100.0f, 100.0f, 150.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Teleporter", meta=(ClampMin=0.0, Units="s"))
	float TeleportCooldown = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Teleporter")
	bool bKeepVelocity = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Teleporter", meta=(ClampMin=0.0, Units="cm/s"))
	float ExitSpeed = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Editor|Palette")
	TSoftObjectPtr<UTexture2D> PaletteIcon;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Editor")
	TObjectPtr<UTextRenderComponent> TeleporterIdText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Editor")
	TObjectPtr<UTextRenderComponent> ExitIdText;
#endif

	TMap<TWeakObjectPtr<AActor>, float> RecentlyTeleportedActors;

	UFUNCTION()
	void OnTriggerVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	bool CanTeleportActor(AActor* Actor) const;
	void MarkActorTeleported(AActor* Actor);
	void TeleportActorFromPointA(AActor* Actor);
	void TeleportActorToResolvedDestination(AActor* Actor, const FVector& DestinationLocation, const FRotator& DestinationRotation, APlatformerTeleporter* DestinationSettingsOwner);
	APlatformerTeleporter* ResolveExitTeleporter() const;
	FVector GetEntryLocation() const;
	FRotator GetEntryRotation() const;
#if WITH_EDITORONLY_DATA
	void RefreshEditorLabels();
	FString GetResolvedExitIdText() const;
#endif
};
