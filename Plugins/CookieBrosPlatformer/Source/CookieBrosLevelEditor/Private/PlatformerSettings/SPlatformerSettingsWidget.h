#pragma once

#include "CoreMinimal.h"
#include "PlatformerSettings/PlatformerSettingsObjects.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SCompoundWidget.h"

class IDetailsView;
struct FPropertyChangedEvent;

class SPlatformerSettingsWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPlatformerSettingsWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SPlatformerSettingsWidget() override;

private:
	void HandleEditorSelectionChanged(UObject* ChangedObject);
	void HandleSettingsFinishedChanging(const FPropertyChangedEvent& PropertyChangedEvent);
	void RefreshFromSelection();
	UPlatformerActorSettingsObject* CreateSettingsObjectForActor(class AActor* Actor) const;
	FText GetSelectionSummaryText() const;

	TSharedPtr<IDetailsView> DetailsView;
	TStrongObjectPtr<UPlatformerActorSettingsObject> ActiveSettingsObject;
	FText SelectionSummaryText;
};
