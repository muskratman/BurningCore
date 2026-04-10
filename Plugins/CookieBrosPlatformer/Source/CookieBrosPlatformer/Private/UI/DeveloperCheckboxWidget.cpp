#include "UI/DeveloperCheckboxWidget.h"

#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Containers/AllowShrinking.h"

void UDeveloperCheckboxWidget::SetParameterName(const FText& InParameterName)
{
	ParameterName = InParameterName;
	ApplyDisplayValues();
}

void UDeveloperCheckboxWidget::SetCheckboxValue(bool bInValue)
{
	bCurrentValue = bInValue;
	bHasInitializedValue = true;
	ApplyDisplayValues();
}

bool UDeveloperCheckboxWidget::GetCheckboxValue() const
{
	if (Checkbox_Value)
	{
		return Checkbox_Value->IsChecked();
	}

	return bCurrentValue;
}

void UDeveloperCheckboxWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!bHasInitializedValue)
	{
		bCurrentValue = bDefaultValue;
	}

	ApplyDisplayValues();
}

void UDeveloperCheckboxWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Checkbox_Value)
	{
		Checkbox_Value->OnCheckStateChanged.AddDynamic(this, &UDeveloperCheckboxWidget::HandleCheckboxValueChanged);
	}

	bCurrentValue = bDefaultValue;
	bHasInitializedValue = true;
	ApplyDisplayValues();
}

void UDeveloperCheckboxWidget::HandleCheckboxValueChanged(bool bIsChecked)
{
	if (bIsSynchronizing)
	{
		return;
	}

	bCurrentValue = bIsChecked;
	bHasInitializedValue = true;
	ApplyDisplayValues();
}

void UDeveloperCheckboxWidget::ApplyDisplayValues()
{
	TGuardValue<bool> SynchronizationGuard(bIsSynchronizing, true);

	if (Txt_Name)
	{
		Txt_Name->SetText(ParameterName);
	}

	if (Checkbox_Value)
	{
		Checkbox_Value->SetIsChecked(bCurrentValue);
	}
}
