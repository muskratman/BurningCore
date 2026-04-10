#pragma once

#include "Blueprint/UserWidget.h"
#include "DeveloperCheckboxWidget.generated.h"

class UCheckBox;
class UTextBlock;

UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API UDeveloperCheckboxWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Developer Checkbox")
	void SetParameterName(const FText& InParameterName);

	UFUNCTION(BlueprintCallable, Category="Developer Checkbox")
	void SetCheckboxValue(bool bInValue);

	UFUNCTION(BlueprintPure, Category="Developer Checkbox")
	bool GetCheckboxValue() const;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Txt_Name;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCheckBox> Checkbox_Value;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Developer Checkbox")
	FText ParameterName = INVTEXT("Param Name");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Developer Checkbox")
	bool bDefaultValue = false;

private:
	UFUNCTION()
	void HandleCheckboxValueChanged(bool bIsChecked);

	void ApplyDisplayValues();

	bool bCurrentValue = false;
	bool bHasInitializedValue = false;
	bool bIsSynchronizing = false;
};
