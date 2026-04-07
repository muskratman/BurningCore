// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Platformer/Environment/PlatformerComponentTransformOverride.h"
#include "PlatformerSoftPlatform.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;
class UPrimitiveComponent;
class UTexture2D;
class ACharacter;

/**
 * A platformer soft platform that the character can jump or drop through.
 */
UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API APlatformerSoftPlatform : public AActor
{
	GENERATED_BODY()

public:
	APlatformerSoftPlatform();
	virtual void Tick(float DeltaTime) override;
	void SetPlatformSize(const FVector& InPlatformSize);

	FORCEINLINE const FVector& GetPlatformSize() const { return PlatformSize; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> MeshLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> PlatformCollisionLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> PlatformCollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> CollisionCheckLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> CollisionCheckBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Soft Platform|Shape")
	FVector PlatformSize = FVector(100.0f, 500.0f, 40.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Soft Platform|Components")
	FPlatformerComponentTransformOffset MeshTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Soft Platform|Components")
	FPlatformerComponentTransformOffset PlatformCollisionTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Soft Platform|Components")
	FPlatformerComponentTransformOffset CollisionCheckTransformOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Soft Platform|Collision", meta=(ClampMin="1.0", Units="cm"))
	float PlatformCollisionHeight = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Soft Platform|Collision", meta=(ClampMin="1.0", Units="cm"))
	float UnderPlatformTriggerHeight = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Soft Platform|Collision", meta=(ClampMin="0.01", Units="s"))
	float JumpThroughIgnoreDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Soft Platform|Collision", meta=(ClampMin="0.01", Units="s"))
	float DropThroughIgnoreDuration = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Soft Platform|Collision", meta=(ClampMin="0.0", Units="cm/s"))
	float DropThroughDownwardSpeed = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Editor|Palette")
	TSoftObjectPtr<UTexture2D> PaletteIcon;

	TSet<TWeakObjectPtr<ACharacter>> CharactersAbovePlatform;
	TSet<TWeakObjectPtr<ACharacter>> CharactersBelowPlatform;
	TMap<TWeakObjectPtr<ACharacter>, float> IgnoredCharactersUntilTime;

	UFUNCTION()
	void OnTopCheckBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTopCheckEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnBottomCheckBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBottomCheckEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	bool IsCharacterStandingOnPlatform(const ACharacter* Character) const;
	bool IsCharacterRequestingDropThrough(const ACharacter* Character) const;
	void ClearInvalidCharacterSet(TSet<TWeakObjectPtr<ACharacter>>& CharacterSet);
	void StartIgnoringCharacter(ACharacter* Character, float IgnoreDuration, bool bForceDownwardDrop);
	void StopIgnoringCharacter(ACharacter* Character);
	void UpdateIgnoredCharacters();
};
