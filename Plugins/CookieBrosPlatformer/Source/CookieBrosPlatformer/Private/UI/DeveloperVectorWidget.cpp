#include "UI/DeveloperVectorWidget.h"

#include "Containers/AllowShrinking.h"
#include "Components/TextBlock.h"
#include "Misc/DefaultValueHelper.h"

void UDeveloperVectorWidget::SetParameterName(const FText& InParameterName)
{
	ParameterName = InParameterName;
	ApplyDisplayValues();
}

void UDeveloperVectorWidget::SetVectorValue(const FVector& InVectorValue)
{
	X = InVectorValue.X;
	Y = InVectorValue.Y;
	Z = InVectorValue.Z;
	ApplyDisplayValues();
}

FVector UDeveloperVectorWidget::GetVectorValue() const
{
	float ParsedX = X;
	float ParsedY = Y;
	float ParsedZ = Z;

	if (Editable_X)
	{
		TryParseTextValue(Editable_X->GetText(), ParsedX);
	}

	if (Editable_Y)
	{
		TryParseTextValue(Editable_Y->GetText(), ParsedY);
	}

	if (Editable_Z)
	{
		TryParseTextValue(Editable_Z->GetText(), ParsedZ);
	}

	return FVector(ParsedX, ParsedY, ParsedZ);
}

void UDeveloperVectorWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	ApplyDisplayValues();
}

void UDeveloperVectorWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Editable_X)
	{
		Editable_X->OnTextCommitted.AddDynamic(this, &UDeveloperVectorWidget::HandleXTextCommitted);
	}

	if (Editable_Y)
	{
		Editable_Y->OnTextCommitted.AddDynamic(this, &UDeveloperVectorWidget::HandleYTextCommitted);
	}

	if (Editable_Z)
	{
		Editable_Z->OnTextCommitted.AddDynamic(this, &UDeveloperVectorWidget::HandleZTextCommitted);
	}

	ApplyDisplayValues();
}

void UDeveloperVectorWidget::HandleXTextCommitted(const FText& InText, ETextCommit::Type CommitMethod)
{
	(void)CommitMethod;

	if (bIsSynchronizing)
	{
		return;
	}

	float ParsedValue = X;
	if (TryParseTextValue(InText, ParsedValue))
	{
		X = ParsedValue;
	}

	ApplyDisplayValues();
}

void UDeveloperVectorWidget::HandleYTextCommitted(const FText& InText, ETextCommit::Type CommitMethod)
{
	(void)CommitMethod;

	if (bIsSynchronizing)
	{
		return;
	}

	float ParsedValue = Y;
	if (TryParseTextValue(InText, ParsedValue))
	{
		Y = ParsedValue;
	}

	ApplyDisplayValues();
}

void UDeveloperVectorWidget::HandleZTextCommitted(const FText& InText, ETextCommit::Type CommitMethod)
{
	(void)CommitMethod;

	if (bIsSynchronizing)
	{
		return;
	}

	float ParsedValue = Z;
	if (TryParseTextValue(InText, ParsedValue))
	{
		Z = ParsedValue;
	}

	ApplyDisplayValues();
}

void UDeveloperVectorWidget::ApplyDisplayValues()
{
	TGuardValue<bool> SynchronizationGuard(bIsSynchronizing, true);

	if (Txt_Name)
	{
		Txt_Name->SetText(ParameterName);
	}

	if (Editable_X)
	{
		Editable_X->SetText(FormatValueAsText(X));
	}

	if (Editable_Y)
	{
		Editable_Y->SetText(FormatValueAsText(Y));
	}

	if (Editable_Z)
	{
		Editable_Z->SetText(FormatValueAsText(Z));
	}
}

FText UDeveloperVectorWidget::FormatValueAsText(float InValue) const
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

bool UDeveloperVectorWidget::TryParseTextValue(const FText& InText, float& OutValue) const
{
	return FDefaultValueHelper::ParseFloat(InText.ToString(), OutValue);
}
