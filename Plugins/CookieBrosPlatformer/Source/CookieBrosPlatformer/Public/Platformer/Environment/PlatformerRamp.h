#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Platformer/Environment/PlatformerComponentTransformOverride.h"
#include "PlatformerRamp.generated.h"

class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UTexture2D;

USTRUCT(BlueprintType)
struct COOKIEBROSPLATFORMER_API FPlatformerRampMeshVariant
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ramp")
	int32 AngleDegrees = 45;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ramp")
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;
};

/**
 * Simple sloped ramp block with adjustable size and incline for blockout use.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerRamp : public AActor
{
	GENERATED_BODY()

public:
	APlatformerRamp();

	void SetRampSize(const FVector& InRampSize);
	void SetRampAngleDegrees(int32 InRampAngleDegrees);

	FORCEINLINE const FVector& GetRampSize() const { return RampSize; }
	FORCEINLINE int32 GetRampAngleDegrees() const { return RampAngleDegrees; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	UStaticMesh* ResolveRampStaticMesh() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> RampMeshLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> RampMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ramp|Shape")
	FVector RampSize = FVector(300.0f, 200.0f, 50.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ramp|Shape", meta=(ClampMin=1))
	int32 RampAngleDegrees = 45;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ramp|Shape", meta=(ClampMin=-89.0, ClampMax=89.0, Units="deg"))
	float SlopePitch = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ramp|Shape", meta=(ClampMin=-89.0, ClampMax=89.0, Units="deg"))
	float SlopeRoll = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ramp|Visual")
	TObjectPtr<UStaticMesh> DefaultRampMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ramp|Visual")
	TArray<FPlatformerRampMeshVariant> RampMeshVariants;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ramp|Components")
	FPlatformerComponentTransformOffset RampMeshTransformOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Editor|Palette")
	TSoftObjectPtr<UTexture2D> PaletteIcon;
};
