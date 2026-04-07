#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlatformerGrapplePoint.generated.h"

class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;
class UTexture2D;

UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerGrapplePoint : public AActor
{
	GENERATED_BODY()

public:
	APlatformerGrapplePoint();

	UFUNCTION(BlueprintPure, Category="Grapple")
	FVector GetGrappleLocation() const;

	UFUNCTION(BlueprintPure, Category="Grapple")
	bool CanBeGrappled() const;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> GrappleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> GrappleAnchor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USphereComponent> DetectionSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grapple")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grapple", meta=(ClampMin=0.0, Units="cm"))
	float MaxGrappleDistance = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grapple", meta=(ClampMin=1.0, Units="cm"))
	float DetectionRadius = 48.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Editor|Palette")
	TSoftObjectPtr<UTexture2D> PaletteIcon;
};
