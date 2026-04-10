#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlatformerHealthWidget.generated.h"

class UProgressBar;

/**
 * Lightweight world-attached health bar widget used by combatants.
 * Presentation is expected to come from a BP-derived widget with a ProgressBar named ProgressBar_Health.
 */
UCLASS(Abstract)
class COOKIEBROSPLATFORMER_API UPlatformerHealthWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Health")
	void SetHealthValues(float CurrentHealth, float MaxHealth);

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_Health;

private:
	void RefreshHealthProgressBar();

	float CachedCurrentHealth = 0.0f;
	float CachedMaxHealth = 1.0f;
};
