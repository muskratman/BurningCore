#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Platformer/Environment/PlatformerComponentTransformOverride.h"
#include "PlatformerVanishingBlock.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;
class UStaticMeshComponent;
class UTexture2D;

/**
 * Disappearing platform block that can be overlap-triggered or auto-cycled.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerVanishingBlock : public AActor
{
	GENERATED_BODY()

public:
	APlatformerVanishingBlock();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> BlockMeshLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> BlockMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> TriggerVolumeLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> TriggerVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vanishing Block|Shape")
	FVector BlockSize = FVector(100.0f, 100.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vanishing Block|Components")
	FPlatformerComponentTransformOffset BlockMeshTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vanishing Block|Components")
	FPlatformerComponentTransformOffset TriggerVolumeTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vanishing Block|Behaviour")
	bool bStartVisible = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vanishing Block|Behaviour")
	bool bTriggerOnCharacterOverlap = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vanishing Block|Behaviour")
	bool bAutoCycle = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vanishing Block|Behaviour", meta=(ClampMin=0.0, Units="s"))
	float VisibleDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vanishing Block|Behaviour", meta=(ClampMin=0.0, Units="s"))
	float HiddenDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vanishing Block|Behaviour", meta=(ClampMin=0.0, Units="s"))
	float InitialDelay = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Editor|Palette")
	TSoftObjectPtr<UTexture2D> PaletteIcon;

	bool bCurrentlyVisible = true;
	FTimerHandle StateTimerHandle;

	UFUNCTION()
	void OnTriggerVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void ShowBlock();

	UFUNCTION()
	void HideBlock();

	void SetBlockActive(bool bNewVisible);
};
