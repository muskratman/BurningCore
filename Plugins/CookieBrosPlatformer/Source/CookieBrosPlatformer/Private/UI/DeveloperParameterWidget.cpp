#include "UI/DeveloperParameterWidget.h"

#include "Containers/AllowShrinking.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/DefaultValueHelper.h"

void UDeveloperParameterWidget::SetParameterName(const FText& InParameterName)
{
	ParameterName = InParameterName;

	if (Txt_Name)
	{
		Txt_Name->SetText(ParameterName);
	}
}

void UDeveloperParameterWidget::SetParameterValue(float InValue)
{
	CurrentValue = SanitizeValue(InValue);
	bHasInitializedValue = true;
	SyncWidgetsToCurrentValue();
}

float UDeveloperParameterWidget::GetParameterValue() const
{
	return CurrentValue;
}

float UDeveloperParameterWidget::GetEditableParameterValue() const
{
	if (!Editable_Param)
	{
		return CurrentValue;
	}

	float ParsedValue = CurrentValue;
	if (TryParseTextValue(Editable_Param->GetText(), ParsedValue))
	{
		return SanitizeValue(ParsedValue);
	}

	return CurrentValue;
}

void UDeveloperParameterWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!bHasInitializedValue)
	{
		CurrentValue = SanitizeValue(DefaultValue);
	}

	ApplyParameterMetadata();
	SyncWidgetsToCurrentValue();
}

void UDeveloperParameterWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Slider_Param)
	{
		Slider_Param->OnValueChanged.AddDynamic(this, &UDeveloperParameterWidget::HandleSliderValueChanged);
	}

	if (Editable_Param)
	{
		Editable_Param->OnTextCommitted.AddDynamic(this, &UDeveloperParameterWidget::HandleEditableTextCommitted);
	}

	CurrentValue = SanitizeValue(DefaultValue);
	bHasInitializedValue = true;

	ApplyParameterMetadata();
	SyncWidgetsToCurrentValue();
}

void UDeveloperParameterWidget::HandleSliderValueChanged(float InValue)
{
	if (bIsSynchronizing)
	{
		return;
	}

	CurrentValue = SanitizeValue(InValue);
	bHasInitializedValue = true;
	SyncWidgetsToCurrentValue();
}

void UDeveloperParameterWidget::HandleEditableTextCommitted(const FText& InText, ETextCommit::Type CommitMethod)
{
	(void)CommitMethod;

	if (bIsSynchronizing)
	{
		return;
	}

	float ParsedValue = 0.0f;
	if (TryParseTextValue(InText, ParsedValue))
	{
		CurrentValue = SanitizeValue(ParsedValue);
		bHasInitializedValue = true;
	}

	SyncWidgetsToCurrentValue();
}

void UDeveloperParameterWidget::ApplyParameterMetadata()
{
	if (Txt_Name)
	{
		Txt_Name->SetText(ParameterName);
	}

	if (Slider_Param)
	{
		const float SafeMinValue = FMath::Min(MinValue, MaxValue);
		const float SafeMaxValue = FMath::Max(MinValue, MaxValue);

		Slider_Param->SetMinValue(SafeMinValue);
		Slider_Param->SetMaxValue(SafeMaxValue);
		Slider_Param->SetStepSize(FMath::Max(StepSize, KINDA_SMALL_NUMBER));
	}
}

void UDeveloperParameterWidget::SyncWidgetsToCurrentValue()
{
	TGuardValue<bool> SynchronizationGuard(bIsSynchronizing, true);

	if (Slider_Param)
	{
		Slider_Param->SetValue(CurrentValue);
	}

	if (Editable_Param)
	{
		Editable_Param->SetText(FormatValueAsText(CurrentValue));
	}
}

float UDeveloperParameterWidget::SanitizeValue(float InValue) const
{
	const float SafeMinValue = FMath::Min(MinValue, MaxValue);
	const float SafeMaxValue = FMath::Max(MinValue, MaxValue);
	float SanitizedValue = FMath::Clamp(InValue, SafeMinValue, SafeMaxValue);

	if (StepSize > KINDA_SMALL_NUMBER)
	{
		const float StepsFromMin = FMath::RoundToFloat((SanitizedValue - SafeMinValue) / StepSize);
		SanitizedValue = SafeMinValue + (StepsFromMin * StepSize);
		SanitizedValue = FMath::Clamp(SanitizedValue, SafeMinValue, SafeMaxValue);
	}

	return SanitizedValue;
}

FText UDeveloperParameterWidget::FormatValueAsText(float InValue) const
{
	FString FormattedValue = FString::Printf(TEXT("%.3f"), InValue);

	while (FormattedValue.Contains(TEXT(".")) && FormattedValue.EndsWith(TEXT("0")))
	{
		FormattedValue.LeftChopInline(1, EAllowShrinking::No);
	}

	if (FormattedValue.EndsWith(TEXT(".")))
	{
		FormattedValue.LeftChopInline(1, EAllowShrinking::No);
	}

	return FText::FromString(FormattedValue);
}

bool UDeveloperParameterWidget::TryParseTextValue(const FText& InText, float& OutValue) const
{
	return FDefaultValueHelper::ParseFloat(InText.ToString(), OutValue);
}
