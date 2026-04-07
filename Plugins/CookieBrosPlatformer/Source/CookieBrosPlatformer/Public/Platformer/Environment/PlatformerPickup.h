// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlatformerPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UTexture2D;

/**
 * APlatformerPickup
 * Generic player pickup that delegates reward processing to a runtime service.
 */
UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API APlatformerPickup : public AActor
{
	GENERATED_BODY()

	/** Pickup bounding sphere */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> Sphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> PickupMesh;

public:
	APlatformerPickup();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Editor|Palette")
	TSoftObjectPtr<UTexture2D> PaletteIcon;

	UFUNCTION()
	void BeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION(BlueprintImplementableEvent, Category="Pickup", meta=(DisplayName="On Picked Up"))
	void BP_OnPickedUp();
};
