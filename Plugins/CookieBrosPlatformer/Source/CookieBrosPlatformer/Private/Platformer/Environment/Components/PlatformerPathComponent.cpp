#include "Platformer/Environment/Components/PlatformerPathComponent.h"

#include "DebugRenderSceneProxy.h"
#include "Engine/World.h"

namespace
{
	class FPlatformerPathDebugSceneProxy final : public FDebugRenderSceneProxy
	{
	public:
		explicit FPlatformerPathDebugSceneProxy(const UPrimitiveComponent* InComponent)
			: FDebugRenderSceneProxy(InComponent)
		{
			DrawType = FDebugRenderSceneProxy::WireMesh;
			TextWithoutShadowDistance = 4000.0f;
			bWantsSelectionOutline = false;
		}

		virtual SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = true;
			Result.bDynamicRelevance = true;
			Result.bSeparateTranslucency = true;
			Result.bNormalTranslucency = true;
			return Result;
		}
	};
}

UPlatformerPathComponent::UPlatformerPathComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetHiddenInGame(true);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	bSelectable = false;
}

namespace
{
	FPlatformerPathPoint SanitizePathPoint(const FPlatformerPathPoint& PathPoint)
	{
		FPlatformerPathPoint Result = PathPoint;
		Result.PointDelay = FMath::Max(Result.PointDelay, 0.0f);
		Result.SpeedScale = FMath::Max(Result.SpeedScale, 0.01f);
		return Result;
	}
}

void UPlatformerPathComponent::SetPathPoints(const TArray<FPlatformerPathPoint>& InPathPoints)
{
	PathPoints.Reset(InPathPoints.Num());
	for (const FPlatformerPathPoint& PathPoint : InPathPoints)
	{
		PathPoints.Add(SanitizePathPoint(PathPoint));
	}
	NotifyPathRenderingChanged();
}

void UPlatformerPathComponent::SetPathPoint(int32 PointIndex, const FPlatformerPathPoint& InPathPoint)
{
	if (!PathPoints.IsValidIndex(PointIndex))
	{
		return;
	}

	PathPoints[PointIndex] = SanitizePathPoint(InPathPoint);
	NotifyPathRenderingChanged();
}

void UPlatformerPathComponent::SetPathPointLocation(int32 PointIndex, const FVector& InPointLocation)
{
	if (!PathPoints.IsValidIndex(PointIndex))
	{
		return;
	}

	PathPoints[PointIndex].PointLocation = InPointLocation;
	NotifyPathRenderingChanged();
}

void UPlatformerPathComponent::SetPathPointWorldLocation(int32 PointIndex, const FVector& InWorldLocation)
{
	if (!PathPoints.IsValidIndex(PointIndex))
	{
		return;
	}

	SetPathPointLocation(PointIndex, GetComponentTransform().InverseTransformPosition(InWorldLocation));
}

void UPlatformerPathComponent::SetRepeatPath(bool bInRepeatPath)
{
	bRepeatPath = bInRepeatPath;
	NotifyPathRenderingChanged();
}

void UPlatformerPathComponent::SetPreviewVisible(bool bInShowPreview)
{
	bShowPreview = bInShowPreview;
	NotifyPathRenderingChanged();
}

FVector UPlatformerPathComponent::GetPathPointWorldLocation(int32 PointIndex) const
{
	if (!PathPoints.IsValidIndex(PointIndex))
	{
		return GetComponentLocation();
	}

	return GetComponentTransform().TransformPosition(PathPoints[PointIndex].PointLocation);
}

FVector UPlatformerPathComponent::GetPathPointLocation(int32 PointIndex) const
{
	if (!PathPoints.IsValidIndex(PointIndex))
	{
		return FVector::ZeroVector;
	}

	return PathPoints[PointIndex].PointLocation;
}

FDebugRenderSceneProxy* UPlatformerPathComponent::CreateDebugSceneProxy()
{
#if WITH_EDITOR
	const UWorld* World = GetWorld();
	if (!bShowPreview || PathPoints.IsEmpty() || (World && World->IsGameWorld()))
	{
		return nullptr;
	}

	FPlatformerPathDebugSceneProxy* Proxy = new FPlatformerPathDebugSceneProxy(this);
	const FLinearColor PreviewLinearColor = PathPreviewColor;
	const FColor PreviewColor = PreviewLinearColor.ToFColor(true);
	const FLinearColor TextLinearColor = FLinearColor(PointIndexTextColor);
	const float ResolvedLineThickness = FMath::Max(PathLineThickness, 0.0f);
	const float ResolvedPointRadius = FMath::Max(PointPreviewRadius, 1.0f);
	const int32 SegmentCount = PathPoints.Num() > 1
		? PathPoints.Num() - 1 + (bRepeatPath ? 1 : 0)
		: 0;

	for (int32 SegmentIndex = 0; SegmentIndex < SegmentCount; ++SegmentIndex)
	{
		const FVector StartLocation = GetPathPointWorldLocation(SegmentIndex);
		const FVector EndLocation = GetPathPointWorldLocation((SegmentIndex + 1) % PathPoints.Num());
		Proxy->Lines.Add(FDebugRenderSceneProxy::FDebugLine(StartLocation, EndLocation, PreviewColor, ResolvedLineThickness));
	}

	for (int32 PointIndex = 0; PointIndex < PathPoints.Num(); ++PointIndex)
	{
		const FVector PointLocation = GetPathPointWorldLocation(PointIndex);
		Proxy->Spheres.Add(FDebugRenderSceneProxy::FSphere(ResolvedPointRadius, PointLocation, PreviewLinearColor));
		Proxy->Texts.Add(FDebugRenderSceneProxy::FText3d(
			FString::FromInt(PointIndex),
			PointLocation + FVector(0.0f, 0.0f, ResolvedPointRadius * 2.5f),
			TextLinearColor));
	}

	return Proxy;
#else
	return nullptr;
#endif
}

FBoxSphereBounds UPlatformerPathComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBox LocalBounds(ForceInit);
	const float ResolvedPointRadius = FMath::Max(PointPreviewRadius, 1.0f);
	for (const FPlatformerPathPoint& PathPoint : PathPoints)
	{
		LocalBounds += PathPoint.PointLocation - FVector(ResolvedPointRadius);
		LocalBounds += PathPoint.PointLocation + FVector(ResolvedPointRadius);
	}

	if (!LocalBounds.IsValid)
	{
		LocalBounds = FBox(FVector(-ResolvedPointRadius), FVector(ResolvedPointRadius));
	}

	return FBoxSphereBounds(LocalBounds.TransformBy(LocalToWorld));
}

void UPlatformerPathComponent::NotifyPathRenderingChanged()
{
	UpdateBounds();
	if (IsRegistered())
	{
		MarkRenderStateDirty();
	}
}
