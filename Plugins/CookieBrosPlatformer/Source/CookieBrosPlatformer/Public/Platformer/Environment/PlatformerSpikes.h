#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Platformer/Environment/PlatformerComponentTransformOverride.h"
#include "PlatformerSpikes.generated.h"

class UBoxComponent;
class UGameplayEffect;
class UPrimitiveComponent;
class USceneComponent;
class UStaticMeshComponent;
class UTexture2D;

/**
 * Basic spike hazard with configurable damage and knockback.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerSpikes : public AActor
{
	GENERATED_BODY()

public:
	APlatformerSpikes();

	virtual void Tick(float DeltaTime) override;
	void SetSpikeSize(const FVector& InSpikeSize);

	FORCEINLINE const FVector& GetSpikeSize() const { return SpikeSize; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> MeshLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> DamageVolumeLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> DamageVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spikes|Shape")
	FVector SpikeSize = FVector(160.0f, 500.0f, 40.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spikes|Components")
	FPlatformerComponentTransformOffset MeshTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spikes|Components")
	FPlatformerComponentTransformOffset DamageVolumeTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spikes|Damage", meta=(ClampMin=0.0))
	float DamageAmount = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spikes|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spikes|Damage", meta=(ClampMin=0.0, Units="s"))
	float DamageCooldown = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spikes|Damage")
	bool bLaunchCharacters = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spikes|Damage", meta=(ClampMin=0.0, Units="cm/s"))
	float HorizontalKnockback = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spikes|Damage", meta=(ClampMin=0.0, Units="cm/s"))
	float VerticalKnockback = 275.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Editor|Palette")
	TSoftObjectPtr<UTexture2D> PaletteIcon;

	TSet<TWeakObjectPtr<AActor>> OverlappingActors;
	TMap<TWeakObjectPtr<AActor>, float> LastDamageTimeByActor;

	UFUNCTION()
	void OnDamageVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnDamageVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void TryDamageActor(AActor* OtherActor);
	bool CanDamageActor(AActor* OtherActor) const;
};
