#include "MainMenuWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Spacer.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "AssetRegistry/AssetRegistryModule.h"

TSharedRef<SWidget> UMainMenuWidget::RebuildWidget()
{
	// Root canvas
	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	// Vertical box for layout (centered)
	UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainVBox"));
	UCanvasPanelSlot* VBoxSlot = RootCanvas->AddChildToCanvas(VBox);
	VBoxSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
	VBoxSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	VBoxSlot->SetSize(FVector2D(400.0f, 300.0f));
	VBoxSlot->SetPosition(FVector2D(0.0f, 0.0f));

	// Title
	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	TitleText->SetText(FText::FromString(TEXT("BURNING CORE")));
	FSlateFontInfo TitleFont = TitleText->GetFont();
	TitleFont.Size = 36;
	TitleText->SetFont(TitleFont);
	TitleText->SetJustification(ETextJustify::Center);
	UVerticalBoxSlot* TitleSlot = VBox->AddChildToVerticalBox(TitleText);
	TitleSlot->SetHorizontalAlignment(HAlign_Center);
	TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 30.0f));

	// Spacer
	USpacer* Spacer1 = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), TEXT("Spacer1"));
	Spacer1->SetSize(FVector2D(0.0f, 20.0f));
	VBox->AddChildToVerticalBox(Spacer1);

	// "Select Level" label
	UTextBlock* SelectLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SelectLabel"));
	SelectLabel->SetText(FText::FromString(TEXT("Select Level:")));
	FSlateFontInfo LabelFont = SelectLabel->GetFont();
	LabelFont.Size = 16;
	SelectLabel->SetFont(LabelFont);
	UVerticalBoxSlot* LabelSlot = VBox->AddChildToVerticalBox(SelectLabel);
	LabelSlot->SetHorizontalAlignment(HAlign_Center);
	LabelSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));

	// ComboBox for levels
	LevelComboBox = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), TEXT("LevelComboBox"));
	UVerticalBoxSlot* ComboSlot = VBox->AddChildToVerticalBox(LevelComboBox);
	ComboSlot->SetHorizontalAlignment(HAlign_Fill);
	ComboSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 20.0f));

	// Spacer
	USpacer* Spacer2 = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), TEXT("Spacer2"));
	Spacer2->SetSize(FVector2D(0.0f, 10.0f));
	VBox->AddChildToVerticalBox(Spacer2);

	// Start Game button
	StartGameButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("StartGameButton"));
	StartButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StartButtonText"));
	StartButtonText->SetText(FText::FromString(TEXT("Start Game")));
	FSlateFontInfo ButtonFont = StartButtonText->GetFont();
	ButtonFont.Size = 20;
	StartButtonText->SetFont(ButtonFont);
	StartButtonText->SetJustification(ETextJustify::Center);
	StartGameButton->AddChild(StartButtonText);
	UVerticalBoxSlot* ButtonSlot = VBox->AddChildToVerticalBox(StartGameButton);
	ButtonSlot->SetHorizontalAlignment(HAlign_Fill);

	return Super::RebuildWidget();
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartGameButton)
	{
		StartGameButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnStartGameClicked);
	}

	PopulateLevels();
}

void UMainMenuWidget::OnStartGameClicked()
{
	if (LevelComboBox && LevelComboBox->GetSelectedOption().Len() > 0)
	{
		FString SelectedLevel = LevelComboBox->GetSelectedOption();
		// Build full game path: /Game/Levels/LevelName
		FString LevelPath = FString::Printf(TEXT("/Game/Levels/%s"), *SelectedLevel);
		UGameplayStatics::OpenLevel(this, FName(*LevelPath));
	}
}

void UMainMenuWidget::PopulateLevels()
{
	if (!LevelComboBox)
	{
		return;
	}

	LevelComboBox->ClearOptions();

	// Use AssetRegistry to find maps — works in both editor and packaged builds
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> AssetDataArray;
	AssetRegistry.GetAssetsByPath(FName(TEXT("/Game/Levels")), AssetDataArray, /*bRecursive=*/ true);

	for (const FAssetData& AssetData : AssetDataArray)
	{
		// Filter only World assets (maps)
		if (AssetData.AssetClassPath == UWorld::StaticClass()->GetClassPathName())
		{
			FString MapName = AssetData.AssetName.ToString();
			LevelComboBox->AddOption(MapName);
		}
	}

	if (LevelComboBox->GetOptionCount() > 0)
	{
		LevelComboBox->SetSelectedIndex(0);
	}
}
