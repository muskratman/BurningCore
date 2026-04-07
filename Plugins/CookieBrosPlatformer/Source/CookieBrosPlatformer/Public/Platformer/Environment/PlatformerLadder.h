#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Platformer/Environment/PlatformerComponentTransformOverride.h"
#include "PlatformerLadder.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;
class UStaticMeshComponent;
class UTexture2D;
class APlatformerCharacterBase;

/**
 * Ladder volume that temporarily places characters into climb-friendly movement.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerLadder : public AActor
{
	GENERATED_BODY()

public:
	APlatformerLadder();
	void SetLadderSize(const FVector& InLadderSize);

	FORCEINLINE const FVector& GetLadderSize() const { return LadderSize; }
	FORCEINLINE float GetClimbGravityScale() const { return ClimbGravityScale; }
	FORCEINLINE bool UsesFlyingMovementMode() const { return bUseFlyingMovementMode; }
	FORCEINLINE bool ShouldSnapCharacterDepthToLadder() const { return bSnapCharacterDepthToLadder; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> LadderMeshLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> LadderMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> ClimbVolumeLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> ClimbVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Shape")
	FVector LadderSize = FVector(100.0f, 120.0f, 400.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Components")
	FPlatformerComponentTransformOffset LadderMeshTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Components")
	FPlatformerComponentTransformOffset ClimbVolumeTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Climb")
	float ClimbGravityScale = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Climb")
	bool bUseFlyingMovementMode = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Climb")
	bool bSnapCharacterDepthToLadder = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Editor|Palette")
	TSoftObjectPtr<UTexture2D> PaletteIcon;

	UFUNCTION()
	void OnClimbVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnClimbVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
