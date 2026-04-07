#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "DeveloperParameterWidget.generated.h"

class UEditableTextBox;
class USlider;
class UTextBlock;

UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API UDeveloperParameterWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Developer Parameter")
	void SetParameterName(const FText& InParameterName);

	UFUNCTION(BlueprintCallable, Category="Developer Parameter")
	void SetParameterValue(float InValue);

	UFUNCTION(BlueprintPure, Category="Developer Parameter")
	float GetParameterValue() const;

	UFUNCTION(BlueprintPure, Category="Developer Parameter")
	float GetEditableParameterValue() const;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Txt_Name;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USlider> Slider_Param;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> Editable_Param;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Developer Parameter")
	FText ParameterName = INVTEXT("Parameter Name");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Developer Parameter")
	float DefaultValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Developer Parameter")
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Developer Parameter")
	float MaxValue = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Developer Parameter", meta=(ClampMin=0.0))
	float StepSize = 1.0f;

private:
	UFUNCTION()
	void HandleSliderValueChanged(float InValue);

	UFUNCTION()
	void HandleEditableTextCommitted(const FText& InText, ETextCommit::Type CommitMethod);

	void ApplyParameterMetadata();
	void SyncWidgetsToCurrentValue();
	float SanitizeValue(float InValue) const;
	FText FormatValueAsText(float InValue) const;
	bool TryParseTextValue(const FText& InText, float& OutValue) const;

	float CurrentValue = 0.0f;
	bool bHasInitializedValue = false;
	bool bIsSynchronizing = false;
};
