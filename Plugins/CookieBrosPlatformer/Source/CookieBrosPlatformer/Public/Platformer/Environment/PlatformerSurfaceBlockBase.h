#pragma once

#include "CoreMinimal.h"
#include "Platformer/Environment/PlatformerBlock.h"
#include "PlatformerSurfaceBlockBase.generated.h"

class ACharacter;
class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;

UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API APlatformerSurfaceBlockBase : public APlatformerBlock
{
	GENERATED_BODY()

public:
	APlatformerSurfaceBlockBase();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void HandleCharacterEnteredSurface(ACharacter* Character);
	virtual void HandleCharacterLeftSurface(ACharacter* Character);
	void ClearInvalidOverlappingCharacters();
	bool IsCharacterStandingOnSurface(const ACharacter* Character) const;

	FORCEINLINE const TSet<TWeakObjectPtr<ACharacter>>& GetOverlappingCharacters() const { return OverlappingCharacters; }
	FORCEINLINE UBoxComponent* GetSurfaceVolume() const { return SurfaceVolume; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> SurfaceVolumeLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> SurfaceVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Surface Block|Trigger")
	FVector2D SurfaceCoverage = FVector2D(0.95f, 0.95f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Surface Block|Trigger", meta=(ClampMin=1.0, Units="cm"))
	float SurfaceTriggerHeight = 32.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Surface Block|Components")
	FPlatformerComponentTransformOffset SurfaceVolumeTransformOffset;

	TSet<TWeakObjectPtr<ACharacter>> OverlappingCharacters;

	UFUNCTION()
	void OnSurfaceVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSurfaceVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
