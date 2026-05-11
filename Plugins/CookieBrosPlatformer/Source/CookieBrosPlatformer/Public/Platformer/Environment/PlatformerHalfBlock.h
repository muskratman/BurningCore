#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Platformer/Environment/PlatformerComponentTransformOverride.h"
#include "PlatformerHalfBlock.generated.h"

class ACharacter;
class UBoxComponent;
class UPlatformerDropThroughPlatformComponent;
class USceneComponent;
class UStaticMeshComponent;
class UTexture2D;

/**
 * Static half-block platform with one-way character pass-through behavior.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerHalfBlock : public AActor
{
	GENERATED_BODY()

public:
	APlatformerHalfBlock();

	void SetPlatformSize(const FVector& InPlatformSize);
	void SetHalfTop(bool bInHalfTop);

	UFUNCTION(BlueprintCallable, Category="Half Block|Collision")
	bool RequestCharacterDropThrough(ACharacter* Character);

	FORCEINLINE const FVector& GetPlatformSize() const { return PlatformSize; }
	FORCEINLINE bool IsHalfTop() const { return bHalfTop; }
	FORCEINLINE UStaticMeshComponent* GetPlatformMesh() const { return PlatformMesh; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> PlatformMeshLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> PlatformMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> DropThroughLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> DropThroughTopCheckLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> DropThroughTopCheckBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> DropThroughBottomCheckLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> DropThroughBottomCheckBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UPlatformerDropThroughPlatformComponent> DropThroughPlatformComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Half Block|Shape")
	FVector PlatformSize = FVector(100.0f, 100.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Half Block|Shape")
	bool bHalfTop = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Half Block|Visual", meta=(ClampMin="0.01", ClampMax="1.0"))
	float HalfBlockHeightRatio = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Half Block|Components")
	FPlatformerComponentTransformOffset PlatformMeshTransformOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Editor|Palette")
	TSoftObjectPtr<UTexture2D> PaletteIcon;

	void RefreshHalfBlockLayout();
	float GetResolvedHalfBlockHeight() const;
	float GetResolvedHalfBlockBottomZ() const;
};
