#pragma once

#include "CoreMinimal.h"
#include "Debug/DebugDrawComponent.h"
#include "PlatformerPathComponent.generated.h"

class FDebugRenderSceneProxy;

USTRUCT(BlueprintType)
struct COOKIEBROSPLATFORMER_API FPlatformerPathPoint
{
	GENERATED_BODY()

	FPlatformerPathPoint() = default;

	explicit FPlatformerPathPoint(const FVector& InPointLocation)
		: PointLocation(InPointLocation)
	{
	}

	FPlatformerPathPoint(const FVector& InPointLocation, float InPointDelay, float InSpeedScale)
		: PointLocation(InPointLocation)
		, PointDelay(InPointDelay)
		, SpeedScale(InSpeedScale)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Platformer Path", meta=(MakeEditWidget=true))
	FVector PointLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Platformer Path", meta=(ClampMin="0.0", Units="s"))
	float PointDelay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Platformer Path", meta=(ClampMin="0.01"))
	float SpeedScale = 1.0f;
};

/**
 * Reusable local-space path definition for platformer foundation actors.
 * Owns a single editor-only debug render proxy, so actors do not need child preview components.
 */
UCLASS(ClassGroup=(Platformer), meta=(BlueprintSpawnableComponent))
class COOKIEBROSPLATFORMER_API UPlatformerPathComponent : public UDebugDrawComponent
{
	GENERATED_BODY()

public:
	UPlatformerPathComponent();

	UFUNCTION(BlueprintPure, Category="Platformer Path")
	const TArray<FPlatformerPathPoint>& GetPathPoints() const { return PathPoints; }

	UFUNCTION(BlueprintCallable, Category="Platformer Path")
	void SetPathPoints(const TArray<FPlatformerPathPoint>& InPathPoints);

	UFUNCTION(BlueprintCallable, Category="Platformer Path")
	void SetPathPoint(int32 PointIndex, const FPlatformerPathPoint& InPathPoint);

	UFUNCTION(BlueprintCallable, Category="Platformer Path")
	void SetPathPointLocation(int32 PointIndex, const FVector& InPointLocation);

	UFUNCTION(BlueprintCallable, Category="Platformer Path")
	void SetPathPointWorldLocation(int32 PointIndex, const FVector& InWorldLocation);

	UFUNCTION(BlueprintCallable, Category="Platformer Path")
	void SetRepeatPath(bool bInRepeatPath);

	UFUNCTION(BlueprintPure, Category="Platformer Path")
	bool IsValidPathPointIndex(int32 PointIndex) const { return PathPoints.IsValidIndex(PointIndex); }

	UFUNCTION(BlueprintCallable, Category="Platformer Path")
	void SetPreviewVisible(bool bInShowPreview);

	UFUNCTION(BlueprintPure, Category="Platformer Path")
	bool ShouldRepeatPath() const { return bRepeatPath; }

	UFUNCTION(BlueprintPure, Category="Platformer Path")
	bool ShouldShowPreview() const { return bShowPreview; }

	UFUNCTION(BlueprintPure, Category="Platformer Path")
	FVector GetPathPointLocation(int32 PointIndex) const;

	UFUNCTION(BlueprintPure, Category="Platformer Path")
	FLinearColor GetPathPreviewColor() const { return PathPreviewColor; }

	UFUNCTION(BlueprintPure, Category="Platformer Path")
	float GetPathLineThickness() const { return PathLineThickness; }

	UFUNCTION(BlueprintPure, Category="Platformer Path")
	float GetPointPreviewRadius() const { return PointPreviewRadius; }

	UFUNCTION(BlueprintPure, Category="Platformer Path")
	FColor GetPointIndexTextColor() const { return PointIndexTextColor; }

	UFUNCTION(BlueprintPure, Category="Platformer Path")
	FVector GetPathPointWorldLocation(int32 PointIndex) const;

	UFUNCTION(BlueprintCallable, Category="Platformer Path")
	void RefreshPath() {}

protected:
	virtual FDebugRenderSceneProxy* CreateDebugSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Platformer Path")
	TArray<FPlatformerPathPoint> PathPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Platformer Path")
	bool bRepeatPath = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Platformer Path|Preview")
	bool bShowPreview = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Platformer Path|Preview", meta=(ClampMin="0.01"))
	float PathLineThickness = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Platformer Path|Preview", meta=(ClampMin="1.0", Units="cm"))
	float PointPreviewRadius = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Platformer Path|Preview")
	FLinearColor PathPreviewColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Platformer Path|Preview")
	FColor PointIndexTextColor = FColor::White;

private:
	void NotifyPathRenderingChanged();
};
