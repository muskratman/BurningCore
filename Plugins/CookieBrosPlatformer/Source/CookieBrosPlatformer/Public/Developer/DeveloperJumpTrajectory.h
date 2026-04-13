#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeveloperJumpTrajectory.generated.h"

class APlatformerCharacterBase;
class UGA_PlatformerJump;
class USceneComponent;
class USplineComponent;

UCLASS()
class COOKIEBROSPLATFORMER_API ADeveloperJumpTrajectory : public AActor
{
	GENERATED_BODY()

public:
	ADeveloperJumpTrajectory();

	virtual void Tick(float DeltaTime) override;

	void InitializeAttachedPreview(APlatformerCharacterBase* InSourceCharacter);
	void SetShowJumpTrajectoryPreview(bool bInShowJumpTrajectoryPreview);
	void RefreshTrajectoryPreview();
	ADeveloperJumpTrajectory* SpawnSnapshotCopy() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USplineComponent> StandingJumpTrajectorySpline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USplineComponent> MaxWalkSpeedJumpTrajectorySpline;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Developer|Jump Preview", meta=(ClampMin=2, ClampMax=64))
	int32 JumpTrajectoryPreviewPointCount = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Developer|Jump Preview", meta=(ClampMin=0.001, ClampMax=0.1, Units="s"))
	float JumpTrajectoryPreviewSimulationStep = 0.008333f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Developer|Jump Preview", meta=(ClampMin=0.1, ClampMax=10.0, Units="s"))
	float JumpTrajectoryPreviewMaxDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Developer|Jump Preview", meta=(ClampMin=0.0))
	float JumpTrajectoryPreviewSplineWidth = 24.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Developer|Jump Preview", meta=(ClampMin=1.0))
	float JumpTrajectoryPreviewLineThickness = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Developer|Jump Preview")
	FLinearColor StandingJumpTrajectoryPreviewColor = FLinearColor(0.15f, 0.85f, 1.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Developer|Jump Preview")
	FLinearColor MaxWalkSpeedJumpTrajectoryPreviewColor = FLinearColor(1.0f, 0.55f, 0.1f, 1.0f);

private:
	const UGA_PlatformerJump* FindGrantedJumpAbility() const;
	void RefreshJumpTrajectorySpline(USplineComponent* TargetSpline, float PreJumpHorizontalSpeed, bool bSimulateForwardInput) const;
	TArray<FVector> BuildJumpTrajectoryPreviewPoints(float PreJumpHorizontalSpeed, bool bSimulateForwardInput) const;
	void ApplyJumpTrajectorySplineStyle();
	void ApplyJumpTrajectorySplineVisibility() const;
	void DrawJumpTrajectoryPreview() const;
	void DrawJumpTrajectorySplinePreview(const USplineComponent* JumpTrajectorySpline, const FColor& PreviewColor) const;
	void CopySplinePointsTo(ADeveloperJumpTrajectory* TargetTrajectory) const;
	static void CopySplinePoints(USplineComponent* TargetSpline, const USplineComponent* SourceSpline);

	TWeakObjectPtr<APlatformerCharacterBase> SourceCharacter;
	bool bShowJumpTrajectoryPreview = false;
	bool bIsSnapshotCopy = false;
};
