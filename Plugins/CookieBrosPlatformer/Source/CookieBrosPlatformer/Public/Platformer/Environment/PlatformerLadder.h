#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Platformer/Environment/PlatformerComponentTransformOverride.h"
#include "PlatformerLadder.generated.h"

class UBoxComponent;
class UHierarchicalInstancedStaticMeshComponent;
class UPrimitiveComponent;
class USceneComponent;
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
	void SetClimbVolumeTransformOffset(const FPlatformerComponentTransformOffset& InTransformOffset);
	void SetSnapCharacterDepthToLadder(bool bInSnapCharacterDepthToLadder);
	virtual bool CanCharacterEnterLadder(const APlatformerCharacterBase* Character, float DesiredClimbInput) const;
	virtual void HandleCharacterEnteredLadder(APlatformerCharacterBase* Character, float DesiredClimbInput);
	virtual void HandleCharacterExitedAtTop(APlatformerCharacterBase* Character);
	/** Called once the character has snapped to the ladder and the FSM enters Mounting.
	 *  Override to apply per-ladder effects at the start of a climb (e.g. disable blocking collision). */
	virtual void OnCharacterMounted(APlatformerCharacterBase* Character);
	/** Called when the FSM transitions to None via bottom / sideways dismount.
	 *  Not called after a TopFinish — HandleCharacterExitedAtTop covers that path. */
	virtual void OnCharacterDismountedBottom(APlatformerCharacterBase* Character);

	FORCEINLINE const FVector& GetLadderSize() const { return LadderSize; }
	FORCEINLINE float GetClimbGravityScale() const { return ClimbGravityScale; }
	FORCEINLINE float GetClimbSpeed() const { return ClimbSpeed; }
	FORCEINLINE float GetJumpClimbSpeed() const { return JumpClimbSpeed; }
	FORCEINLINE float GetJumpClimbDuration() const { return JumpClimbDuration; }
	FORCEINLINE float GetCrouchClimbSpeed() const { return CrouchClimbSpeed; }
	FORCEINLINE float GetCrouchClimbDuration() const { return CrouchClimbDuration; }
	FORCEINLINE float GetLadderMaxFlySpeed() const { return LadderMaxFlySpeed; }
	FORCEINLINE float GetMountDuration() const { return MountDuration; }
	FORCEINLINE float GetTopFinishDuration() const { return TopFinishDuration; }
	FORCEINLINE float GetReentryCooldownDuration() const { return ReentryCooldownDuration; }
	FORCEINLINE float GetHorizontalExitInputThreshold() const { return HorizontalExitInputThreshold; }
	FORCEINLINE bool UsesFlyingMovementMode() const { return bUseFlyingMovementMode; }
	FORCEINLINE bool ShouldSnapCharacterHorizontalToLadder() const { return bSnapCharacterHorizontalToLadder; }
	FORCEINLINE bool ShouldSnapCharacterDepthToLadder() const { return bSnapCharacterDepthToLadder; }
	FORCEINLINE bool ShouldSnapCharacterRotationToLadder() const { return bSnapCharacterRotationToLadder; }
	FORCEINLINE FRotator GetCharacterLadderRotation() const { return CharacterLadderRotation; }
	float GetClimbBottomWorldZ() const;
	float GetClimbTopWorldZ() const;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> LadderMeshLayoutRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> LadderMeshInstances;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Climb", meta=(ClampMin="0.0", Units="cm/s"))
	float ClimbSpeed = 320.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Climb", meta=(ClampMin="0.0", Units="cm/s"))
	float JumpClimbSpeed = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Climb", meta=(ClampMin="0.0", Units="s"))
	float JumpClimbDuration = 0.16f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Climb", meta=(ClampMin="0.0", Units="cm/s"))
	float CrouchClimbSpeed = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Climb", meta=(ClampMin="0.0", Units="s"))
	float CrouchClimbDuration = 0.16f;

	/** Decoupled MaxFlySpeed clamp applied while character is on this ladder.
	 *  Keeps ladder velocity independent from the character's general flying speed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Climb", meta=(ClampMin="0.0", Units="cm/s"))
	float LadderMaxFlySpeed = 1200.0f;

	/** Duration of the start-state animation lock right after mounting. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Animation", meta=(ClampMin="0.0", Units="s"))
	float MountDuration = 0.16f;

	/** Duration of the end-state lock at the top after a successful top dismount. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Animation", meta=(ClampMin="0.0", Units="s"))
	float TopFinishDuration = 0.35f;

	/** Cooldown after dismount before character can re-enter the same/another ladder. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Climb", meta=(ClampMin="0.0", Units="s"))
	float ReentryCooldownDuration = 0.25f;

	/** Minimum normalized horizontal input that dismisses character off the ladder sideways. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Input", meta=(ClampMin="0.0", ClampMax="1.0"))
	float HorizontalExitInputThreshold = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Climb")
	bool bUseFlyingMovementMode = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Climb")
	bool bSnapCharacterHorizontalToLadder = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Climb")
	bool bSnapCharacterDepthToLadder = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Climb")
	bool bSnapCharacterRotationToLadder = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder|Climb", meta=(EditCondition="bSnapCharacterRotationToLadder"))
	FRotator CharacterLadderRotation = FRotator(0.0f, -90.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Editor|Palette")
	TSoftObjectPtr<UTexture2D> PaletteIcon;

	UFUNCTION()
	void OnClimbVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnClimbVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void RefreshLadderLayout();
	void SyncOverlappingCharacters();
};
