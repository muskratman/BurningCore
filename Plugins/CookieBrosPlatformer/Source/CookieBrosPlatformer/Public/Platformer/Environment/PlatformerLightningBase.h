#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlatformerLightningBase.generated.h"

class UDirectionalLightComponent;
class UExponentialHeightFogComponent;
class USceneComponent;
class USkyAtmosphereComponent;
class USkyLightComponent;
class UStaticMeshComponent;
class UVolumetricCloudComponent;

/**
 * Lighting actor that mirrors the default "Basic" UE level setup.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerLightningBase : public AActor
{
	GENERATED_BODY()

public:
	APlatformerLightningBase();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UDirectionalLightComponent> DirectionalLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UExponentialHeightFogComponent> ExponentialHeightFog;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> SkySphereMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UVolumetricCloudComponent> VolumetricCloud;
};
