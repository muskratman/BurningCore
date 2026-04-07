#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SideViewMovementComponent.generated.h"

/**
 * USideViewMovementComponent
 * Custom CMC that locks the player to a 2D plane and adds platformer jumping logic.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API USideViewMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	USideViewMovementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void InitializeComponent() override;

	/** The Y-coordinate to lock the character to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SideView", meta=(Units="cm"))
	float LockedDepthY = 0.0f;

	/** Platformer feel tuning: lighter gravity at apex */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SideView|Jump", meta=(ClampMin=0.0f, ClampMax=2000.0f, Units="cm/s"))
	float JumpApexGravityMultiplier = 0.5f;

	/** Grace period after leaving edge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SideView|Jump", meta=(ClampMin=0.0f, ClampMax=3.0f, Units="s"))
	float CoyoteTime = 0.1f;

	/** Pre-land jump buffer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SideView|Jump", meta=(ClampMin=0.0f, ClampMax=3.0f, Units="s"))
	float JumpBufferTime = 0.15f;

	/** Set a temporary depth lane */
	UFUNCTION(BlueprintCallable, Category="SideView")
	void SetDepthLane(float NewY, float TransitionTime = 0.3f);

	UFUNCTION(BlueprintCallable, Category="SideView|Gravity")
	void SetExternalGravityScaleOverride(float NewGravityScale);

	UFUNCTION(BlueprintCallable, Category="SideView|Gravity")
	void ClearExternalGravityScaleOverride();

	UFUNCTION(BlueprintCallable, Category="SideView|Gravity")
	void SetBaseGravityScale(float NewGravityScale);

	UFUNCTION(BlueprintPure, Category="SideView|Gravity")
	bool HasExternalGravityScaleOverride() const { return bHasExternalGravityScaleOverride; }

	UFUNCTION(BlueprintPure, Category="SideView|Gravity")
	float GetExternalGravityScaleOverride() const { return ExternalGravityScaleOverride; }

	UFUNCTION(BlueprintPure, Category="SideView|Gravity")
	float GetBaseGravityScale() const { return DefaultGravityScale; }

protected:
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

	void EnforceDepthLock(float DeltaTime);

	float TargetDepthY;
	float DepthTransitionSpeed;
	float LastGroundedTime;
	float LastJumpInputTime;
	float DefaultGravityScale;
	bool bHasExternalGravityScaleOverride;
	float ExternalGravityScaleOverride;
};
