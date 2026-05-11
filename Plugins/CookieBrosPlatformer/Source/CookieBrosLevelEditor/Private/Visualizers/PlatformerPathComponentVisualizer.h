#pragma once

#include "ComponentVisualizer.h"

class FPrimitiveDrawInterface;
class FSceneView;
class UActorComponent;
class UPlatformerPathComponent;

struct HPlatformerPathPointProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();

	HPlatformerPathPointProxy(const UActorComponent* InComponent, int32 InPointIndex)
		: HComponentVisProxy(InComponent, HPP_Foreground)
		, PointIndex(InPointIndex)
	{
	}

	int32 PointIndex = INDEX_NONE;
};

class FPlatformerPathComponentVisualizer : public FComponentVisualizer
{
public:
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
	virtual void EndEditing() override;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;
	virtual void TrackingStarted(FEditorViewportClient* InViewportClient) override;
	virtual void TrackingStopped(FEditorViewportClient* InViewportClient, bool bInDidMove) override;
	virtual UActorComponent* GetEditedComponent() const override;

private:
	UPlatformerPathComponent* GetEditedPathComponent() const;

	FComponentPropertyPath EditedPathComponentPath;
	int32 SelectedPointIndex = INDEX_NONE;
	int32 ActiveTransactionIndex = INDEX_NONE;
	bool bMovedSelectedPoint = false;
};
