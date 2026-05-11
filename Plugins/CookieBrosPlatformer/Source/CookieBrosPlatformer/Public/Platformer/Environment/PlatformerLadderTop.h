#pragma once

#include "CoreMinimal.h"
#include "Platformer/Environment/PlatformerLadder.h"
#include "PlatformerLadderTop.generated.h"

class UBoxComponent;
class ACharacter;
class UPlatformerDropThroughPlatformComponent;
class USceneComponent;
class UStaticMeshComponent;

/**
 * Ladder variant with a walkable quarter-height cap.
 * The top entry volume exposes the ladder while the character stands on the cap,
 * so pressing down/crouch can enter the existing ladder climb flow.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerLadderTop : public APlatformerLadder
{
	GENERATED_BODY()

public:
	APlatformerLadderTop();

	void SetTopBlockDepth(float InTopBlockDepth);

	UFUNCTION(BlueprintCallable, Category="Ladder Top|Components")
	void SetTopSectionEnabled(bool bInTopSectionEnabled);

	UFUNCTION(BlueprintPure, Category="Ladder Top|Components")
	bool IsTopSectionEnabled() const { return bTopSectionEnabled; }

	virtual bool CanCharacterEnterLadder(const APlatformerCharacterBase* Character, float DesiredClimbInput) const override;
	virtual void HandleCharacterEnteredLadder(APlatformerCharacterBase* Character, float DesiredClimbInput) override;
	virtual void HandleCharacterExitedAtTop(APlatformerCharacterBase* Character) override;
	/** Disables TopBlock blocking for this character so they can climb through it. */
	virtual void OnCharacterMounted(APlatformerCharacterBase* Character) override;
	/** Restores TopBlock blocking after a bottom or sideways dismount. */
	virtual void OnCharacterDismountedBottom(APlatformerCharacterBase* Character) override;

	UFUNCTION(BlueprintCallable, Category="Ladder Top|Collision")
	bool RequestCharacterDropThrough(ACharacter* Character);

protected:
	virtual void RefreshLadderLayout() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> TopBlockMeshLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> TopBlockMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> TopBlockDropThroughLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> TopBlockDropThroughTopCheckLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> TopBlockDropThroughTopCheckBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> TopBlockDropThroughBottomCheckLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> TopBlockDropThroughBottomCheckBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UPlatformerDropThroughPlatformComponent> TopBlockDropThroughPlatformComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> TopEntryVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder Top|Shape", meta=(ClampMin="0.01"))
	float TopBlockHeightRatio = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder Top|Shape", meta=(ClampMin="1.0", Units="cm"))
	float TopBlockDepth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder Top|Shape", meta=(ClampMin="1.0", Units="cm"))
	float TopEntryVolumeHeight = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder Top|Shape", meta=(ClampMin="0.0", Units="cm"))
	float TopBlockFeetTolerance = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder Top|Components")
	bool bTopSectionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder Top|Components")
	FPlatformerComponentTransformOffset TopBlockMeshTransformOffset;

private:
	void ApplyTopSectionEnabled();
	bool IsCharacterOnTopBlock(const APlatformerCharacterBase* Character) const;
	float GetResolvedTopBlockHeight(const FVector& ResolvedLadderSize) const;
	void RefreshTopLadderMeshLayout(const FVector& ResolvedLadderSize);
	void RefreshTopLadderClimbVolumeLayout(const FVector& ResolvedLadderSize);

	UFUNCTION()
	void OnTopEntryVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTopEntryVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
