#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Platformer/Environment/PlatformerComponentTransformOverride.h"
#include "PlatformerBlock.generated.h"

class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UTexture2D;

UENUM(BlueprintType)
enum class EPlatformerBlockMeshVariant : uint8
{
	FullSize,
	HalfSize
};

/**
 * Simple cubic block with adjustable size for platformer blockout use.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerBlock : public AActor
{
	GENERATED_BODY()

public:
	APlatformerBlock();

	void SetBlockSize(const FVector& InBlockSize);
	void SetBlockMeshVariant(EPlatformerBlockMeshVariant InBlockMeshVariant);

	FORCEINLINE const FVector& GetBlockSize() const { return BlockSize; }
	FORCEINLINE EPlatformerBlockMeshVariant GetBlockMeshVariant() const { return BlockMeshVariant; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	UStaticMesh* ResolveBlockStaticMesh() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> BlockMeshLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> BlockMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Block|Shape")
	FVector BlockSize = FVector(100.0f, 100.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Block|Visual")
	EPlatformerBlockMeshVariant BlockMeshVariant = EPlatformerBlockMeshVariant::FullSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Block|Visual")
	TObjectPtr<UStaticMesh> FullSizeMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Block|Visual")
	TObjectPtr<UStaticMesh> HalfSizeMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Block|Components")
	FPlatformerComponentTransformOffset BlockMeshTransformOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Editor|Palette")
	TSoftObjectPtr<UTexture2D> PaletteIcon;
};
