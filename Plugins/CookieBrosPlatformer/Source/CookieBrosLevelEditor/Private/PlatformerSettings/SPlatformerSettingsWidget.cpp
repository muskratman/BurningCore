#include "PlatformerSettings/SPlatformerSettingsWidget.h"

#include "Platformer/Environment/PlatformerConveyor.h"
#include "Platformer/Environment/PlatformerDangerBlock.h"
#include "Platformer/Environment/PlatformerDestructibleBlock.h"
#include "Platformer/Environment/PlatformerFallingPlatform.h"
#include "Platformer/Environment/PlatformerGravityVolume.h"
#include "Platformer/Environment/PlatformerHazardProjectile.h"
#include "Platformer/Environment/PlatformerJumpPad.h"
#include "Platformer/Environment/PlatformerMovingPlatform.h"
#include "Platformer/Environment/PlatformerSlipperyBlock.h"
#include "Platformer/Environment/PlatformerSpikes.h"
#include "Platformer/Environment/PlatformerStream.h"
#include "Platformer/Environment/PlatformerSwitch.h"
#include "Platformer/Environment/PlatformerTeleporter.h"
#include "Platformer/Environment/PlatformerTriggeredLift.h"
#include "Platformer/Environment/PlatformerVanishingBlock.h"
#include "Platformer/Environment/PlatformerWallTurret.h"
#include "PlatformerSettings/PlatformerSettingsObjects.h"
#include "Editor.h"
#include "Engine/Selection.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "ScopedTransaction.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SPlatformerSettingsWidget"

void SPlatformerSettingsWidget::Construct(const FArguments& InArgs)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.bShowPropertyMatrixButton = false;
	DetailsViewArgs.bShowObjectLabel = false;
	DetailsViewArgs.bUpdatesFromSelection = false;

	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->OnFinishedChangingProperties().AddSP(this, &SPlatformerSettingsWidget::HandleSettingsFinishedChanging);

	SelectionSummaryText = LOCTEXT("SelectionPrompt", "Select one supported Platformer Environment actor to edit its quick settings.");

	ChildSlot
	[
		SNew(SBorder)
		.Padding(8.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(this, &SPlatformerSettingsWidget::GetSelectionSummaryText)
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				DetailsView.ToSharedRef()
			]
		]
	];

	USelection::SelectionChangedEvent.AddSP(this, &SPlatformerSettingsWidget::HandleEditorSelectionChanged);
	USelection::SelectObjectEvent.AddSP(this, &SPlatformerSettingsWidget::HandleEditorSelectionChanged);

	RefreshFromSelection();
}

SPlatformerSettingsWidget::~SPlatformerSettingsWidget()
{
	USelection::SelectionChangedEvent.RemoveAll(this);
	USelection::SelectObjectEvent.RemoveAll(this);
}

void SPlatformerSettingsWidget::HandleEditorSelectionChanged(UObject* ChangedObject)
{
	RefreshFromSelection();
}

void SPlatformerSettingsWidget::HandleSettingsFinishedChanging(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (!ActiveSettingsObject.IsValid())
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("UpdatePlatformerSettingsTransaction", "Update Platformer Settings"));
	ActiveSettingsObject->PushToActor();
	RefreshFromSelection();
}

void SPlatformerSettingsWidget::RefreshFromSelection()
{
	if (GEditor == nullptr)
	{
		ActiveSettingsObject.Reset();
		SelectionSummaryText = LOCTEXT("EditorUnavailable", "Editor selection is not available.");
		DetailsView->SetObject(nullptr);
		return;
	}

	TArray<AActor*> SelectedActors;
	GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(SelectedActors);

	if (SelectedActors.Num() != 1)
	{
		ActiveSettingsObject.Reset();
		SelectionSummaryText = LOCTEXT("SingleSelectionPrompt", "Select exactly one supported Platformer Environment actor.");
		DetailsView->SetObject(nullptr);
		return;
	}

	AActor* SelectedActor = SelectedActors[0];
	if (SelectedActor == nullptr)
	{
		ActiveSettingsObject.Reset();
		SelectionSummaryText = LOCTEXT("InvalidSelection", "Select one supported Platformer Environment actor.");
		DetailsView->SetObject(nullptr);
		return;
	}

	ActiveSettingsObject.Reset(CreateSettingsObjectForActor(SelectedActor));
	if (!ActiveSettingsObject.IsValid())
	{
		SelectionSummaryText = FText::Format(
			LOCTEXT("UnsupportedActor", "{0} is not supported by PlatformerSettings."),
			FText::FromString(SelectedActor->GetClass()->GetName()));
		DetailsView->SetObject(nullptr);
		return;
	}

	ActiveSettingsObject->PullFromActor(SelectedActor);
	SelectionSummaryText = FText::Format(
		LOCTEXT("SelectionSummary", "{0} ({1})"),
		FText::FromString(SelectedActor->GetActorLabel()),
		FText::FromString(SelectedActor->GetClass()->GetName()));

	DetailsView->SetObject(ActiveSettingsObject.Get(), true);
	DetailsView->ForceRefresh();
}

UPlatformerActorSettingsObject* SPlatformerSettingsWidget::CreateSettingsObjectForActor(AActor* Actor) const
{
	if (Actor == nullptr)
	{
		return nullptr;
	}

	if (Actor->IsA<APlatformerTriggeredLift>())
	{
		return NewObject<UPlatformerTriggeredLiftSettingsObject>(GetTransientPackage());
	}

	if (Actor->IsA<APlatformerMovingPlatform>())
	{
		return NewObject<UPlatformerMovingPlatformSettingsObject>(GetTransientPackage());
	}

	if (Actor->IsA<APlatformerConveyor>())
	{
		return NewObject<UPlatformerConveyorSettingsObject>(GetTransientPackage());
	}

	if (Actor->IsA<APlatformerDangerBlock>())
	{
		return NewObject<UPlatformerDangerBlockSettingsObject>(GetTransientPackage());
	}

	if (Actor->IsA<APlatformerDestructibleBlock>())
	{
		return NewObject<UPlatformerDestructibleBlockSettingsObject>(GetTransientPackage());
	}

	if (Actor->IsA<APlatformerFallingPlatform>())
	{
		return NewObject<UPlatformerFallingPlatformSettingsObject>(GetTransientPackage());
	}

	if (Actor->IsA<APlatformerGravityVolume>())
	{
		return NewObject<UPlatformerGravityVolumeSettingsObject>(GetTransientPackage());
	}

	if (Actor->IsA<APlatformerHazardProjectile>())
	{
		return NewObject<UPlatformerHazardProjectileSettingsObject>(GetTransientPackage());
	}

	if (Actor->IsA<APlatformerJumpPad>())
	{
		return NewObject<UPlatformerJumpPadSettingsObject>(GetTransientPackage());
	}

	if (Actor->IsA<APlatformerSlipperyBlock>())
	{
		return NewObject<UPlatformerSlipperyBlockSettingsObject>(GetTransientPackage());
	}

	if (Actor->IsA<APlatformerSpikes>())
	{
		return NewObject<UPlatformerSpikesSettingsObject>(GetTransientPackage());
	}

	if (Actor->IsA<APlatformerStream>())
	{
		return NewObject<UPlatformerStreamSettingsObject>(GetTransientPackage());
	}

	if (Actor->IsA<APlatformerSwitch>())
	{
		return NewObject<UPlatformerSwitchSettingsObject>(GetTransientPackage());
	}

	if (Actor->IsA<APlatformerTeleporter>())
	{
		return NewObject<UPlatformerTeleporterSettingsObject>(GetTransientPackage());
	}

	if (Actor->IsA<APlatformerVanishingBlock>())
	{
		return NewObject<UPlatformerVanishingBlockSettingsObject>(GetTransientPackage());
	}

	if (Actor->IsA<APlatformerWallTurret>())
	{
		return NewObject<UPlatformerWallTurretSettingsObject>(GetTransientPackage());
	}

	return nullptr;
}

FText SPlatformerSettingsWidget::GetSelectionSummaryText() const
{
	return SelectionSummaryText;
}

#undef LOCTEXT_NAMESPACE
