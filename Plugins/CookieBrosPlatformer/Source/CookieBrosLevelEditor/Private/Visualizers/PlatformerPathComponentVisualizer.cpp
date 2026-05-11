#include "Visualizers/PlatformerPathComponentVisualizer.h"

#include "Editor.h"
#include "EditorViewportClient.h"
#include "InputCoreTypes.h"
#include "Platformer/Environment/Components/PlatformerPathComponent.h"
#include "PrimitiveDrawInterface.h"
#include "ScopedTransaction.h"
#include "SceneView.h"
#include "UObject/UnrealType.h"

IMPLEMENT_HIT_PROXY(HPlatformerPathPointProxy, HComponentVisProxy);

#define LOCTEXT_NAMESPACE "PlatformerPathComponentVisualizer"

void FPlatformerPathComponentVisualizer::DrawVisualization(
	const UActorComponent* Component,
	const FSceneView* View,
	FPrimitiveDrawInterface* PDI)
{
	const UPlatformerPathComponent* PathComponent = Cast<const UPlatformerPathComponent>(Component);
	if (!PathComponent || !PathComponent->ShouldShowPreview() || !PDI)
	{
		return;
	}

	const TArray<FPlatformerPathPoint>& PathPoints = PathComponent->GetPathPoints();
	const FLinearColor PreviewColor = PathComponent->GetPathPreviewColor();
	const FLinearColor SelectedColor = FLinearColor::Yellow;
	const float PointSize = FMath::Max(PathComponent->GetPointPreviewRadius() * 2.5f, 12.0f);

	for (int32 PointIndex = 0; PointIndex < PathPoints.Num(); ++PointIndex)
	{
		const bool bIsSelected =
			EditedPathComponentPath.GetComponent() == PathComponent &&
			SelectedPointIndex == PointIndex;
		PDI->SetHitProxy(new HPlatformerPathPointProxy(PathComponent, PointIndex));
		PDI->DrawPoint(
			PathComponent->GetPathPointWorldLocation(PointIndex),
			bIsSelected ? SelectedColor : PreviewColor,
			PointSize,
			SDPG_Foreground);
		PDI->SetHitProxy(nullptr);
	}
}

bool FPlatformerPathComponentVisualizer::VisProxyHandleClick(
	FEditorViewportClient* InViewportClient,
	HComponentVisProxy* VisProxy,
	const FViewportClick& Click)
{
	if (!VisProxy || !VisProxy->Component.IsValid() || !VisProxy->IsA(HPlatformerPathPointProxy::StaticGetType()))
	{
		return false;
	}

	const HPlatformerPathPointProxy* PointProxy = static_cast<const HPlatformerPathPointProxy*>(VisProxy);
	UPlatformerPathComponent* PathComponent = Cast<UPlatformerPathComponent>(const_cast<UActorComponent*>(VisProxy->Component.Get()));
	if (!PathComponent || !PathComponent->IsValidPathPointIndex(PointProxy->PointIndex))
	{
		EndEditing();
		return false;
	}

	const FScopedTransaction Transaction(LOCTEXT("SelectPlatformerPathPoint", "Select Platformer Path Point"));
	EditedPathComponentPath = FComponentPropertyPath(PathComponent);
	SelectedPointIndex = PointProxy->PointIndex;
	bMovedSelectedPoint = false;
	return true;
}

void FPlatformerPathComponentVisualizer::EndEditing()
{
	EditedPathComponentPath.Reset();
	SelectedPointIndex = INDEX_NONE;
	ActiveTransactionIndex = INDEX_NONE;
	bMovedSelectedPoint = false;
}

bool FPlatformerPathComponentVisualizer::GetWidgetLocation(
	const FEditorViewportClient* ViewportClient,
	FVector& OutLocation) const
{
	const UPlatformerPathComponent* PathComponent = GetEditedPathComponent();
	if (!PathComponent || !PathComponent->IsValidPathPointIndex(SelectedPointIndex))
	{
		return false;
	}

	OutLocation = PathComponent->GetPathPointWorldLocation(SelectedPointIndex);
	return true;
}

bool FPlatformerPathComponentVisualizer::HandleInputDelta(
	FEditorViewportClient* ViewportClient,
	FViewport* Viewport,
	FVector& DeltaTranslate,
	FRotator& DeltaRotate,
	FVector& DeltaScale)
{
	UPlatformerPathComponent* PathComponent = GetEditedPathComponent();
	if (!PathComponent || !PathComponent->IsValidPathPointIndex(SelectedPointIndex) || DeltaTranslate.IsNearlyZero())
	{
		return false;
	}

	PathComponent->Modify();
	const FVector NewWorldLocation = PathComponent->GetPathPointWorldLocation(SelectedPointIndex) + DeltaTranslate;
	PathComponent->SetPathPointWorldLocation(SelectedPointIndex, NewWorldLocation);

	if (FProperty* PathPointsProperty = FindFProperty<FProperty>(UPlatformerPathComponent::StaticClass(), TEXT("PathPoints")))
	{
		FComponentVisualizer::NotifyPropertyModified(PathComponent, PathPointsProperty, EPropertyChangeType::Interactive);
	}

	bMovedSelectedPoint = true;
	return true;
}

void FPlatformerPathComponentVisualizer::TrackingStarted(FEditorViewportClient* InViewportClient)
{
	UPlatformerPathComponent* PathComponent = GetEditedPathComponent();
	if (GEditor && PathComponent && PathComponent->IsValidPathPointIndex(SelectedPointIndex))
	{
		ActiveTransactionIndex = GEditor->BeginTransaction(LOCTEXT("MovePlatformerPathPoint", "Move Platformer Path Point"));
		PathComponent->Modify();
	}
}

void FPlatformerPathComponentVisualizer::TrackingStopped(FEditorViewportClient* InViewportClient, bool bInDidMove)
{
	UPlatformerPathComponent* PathComponent = GetEditedPathComponent();
	if (PathComponent && bMovedSelectedPoint)
	{
		if (FProperty* PathPointsProperty = FindFProperty<FProperty>(UPlatformerPathComponent::StaticClass(), TEXT("PathPoints")))
		{
			FComponentVisualizer::NotifyPropertyModified(PathComponent, PathPointsProperty, EPropertyChangeType::ValueSet);
		}
	}

	if (GEditor && ActiveTransactionIndex != INDEX_NONE)
	{
		GEditor->EndTransaction();
		ActiveTransactionIndex = INDEX_NONE;
	}

	bMovedSelectedPoint = false;
}

UActorComponent* FPlatformerPathComponentVisualizer::GetEditedComponent() const
{
	return GetEditedPathComponent();
}

UPlatformerPathComponent* FPlatformerPathComponentVisualizer::GetEditedPathComponent() const
{
	return Cast<UPlatformerPathComponent>(EditedPathComponentPath.GetComponent());
}

#undef LOCTEXT_NAMESPACE
