#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Platformer/Environment/PlatformerComponentTransformOverride.h"
#include "PlatformerStream.generated.h"

class ACharacter;
class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;
class UStaticMeshComponent;
class UTexture2D;

USTRUCT()
struct FPlatformerStreamMovementState
{
	GENERATED_BODY()

	UPROPERTY()
	float MaxWalkSpeed = 600.0f;

	UPROPERTY()
	float MaxAcceleration = 2048.0f;

	UPROPERTY()
	float BrakingDecelerationWalking = 2048.0f;
};

UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerStream : public AActor
{
	GENERATED_BODY()

public:
	APlatformerStream();

	virtual void Tick(float DeltaTime) override;
	void SetVolumeSize(const FVector& InVolumeSize);

	FORCEINLINE const FVector& GetVolumeSize() const { return VolumeSize; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> VolumeMeshLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> VolumeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> VolumeLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> Volume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stream|Shape")
	FVector VolumeSize = FVector(100.0f, 500.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stream|Components")
	FPlatformerComponentTransformOffset VolumeMeshTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stream|Components")
	FPlatformerComponentTransformOffset VolumeTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stream|Components")
	bool bVolumeMeshUsesBottomPivot = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stream|Movement", meta=(Units="cm/s"))
	FVector LocalFlowVelocity = FVector(250.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stream|Movement", meta=(ClampMin=0.01))
	float MovementSpeedMultiplier = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stream|Movement")
	bool bAffectAirborneCharacters = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Editor|Palette")
	TSoftObjectPtr<UTexture2D> PaletteIcon;

	TMap<TWeakObjectPtr<ACharacter>, FPlatformerStreamMovementState> AffectedCharacters;

	UFUNCTION()
	void OnVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void RestoreCharacter(ACharacter* Character);
};
