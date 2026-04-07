#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "DeveloperVectorWidget.generated.h"

class UEditableTextBox;
class UTextBlock;

UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API UDeveloperVectorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Developer Vector")
	void SetParameterName(const FText& InParameterName);

	UFUNCTION(BlueprintCallable, Category="Developer Vector")
	void SetVectorValue(const FVector& InVectorValue);

	UFUNCTION(BlueprintPure, Category="Developer Vector")
	FVector GetVectorValue() const;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Txt_Name;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> Editable_X;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> Editable_Y;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> Editable_Z;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Developer Vector")
	FText ParameterName = INVTEXT("Parameter Name");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Developer Vector")
	float X = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Developer Vector")
	float Y = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Developer Vector")
	float Z = 0.0f;

private:
	UFUNCTION()
	void HandleXTextCommitted(const FText& InText, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void HandleYTextCommitted(const FText& InText, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void HandleZTextCommitted(const FText& InText, ETextCommit::Type CommitMethod);

	void ApplyDisplayValues();
	FText FormatValueAsText(float InValue) const;
	bool TryParseTextValue(const FText& InText, float& OutValue) const;

	bool bIsSynchronizing = false;
};
