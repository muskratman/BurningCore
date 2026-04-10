#include "UI/PlatformerHealthWidget.h"

#include "Components/ProgressBar.h"

void UPlatformerHealthWidget::SetHealthValues(float CurrentHealth, float MaxHealth)
{
	CachedCurrentHealth = FMath::Max(0.0f, CurrentHealth);
	CachedMaxHealth = FMath::Max(1.0f, MaxHealth);
	RefreshHealthProgressBar();
}

void UPlatformerHealthWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RefreshHealthProgressBar();
}

void UPlatformerHealthWidget::RefreshHealthProgressBar()
{
	if (!ProgressBar_Health)
	{
		return;
	}

	const float HealthPercent = CachedMaxHealth > KINDA_SMALL_NUMBER
		? FMath::Clamp(CachedCurrentHealth / CachedMaxHealth, 0.0f, 1.0f)
		: 0.0f;

	ProgressBar_Health->SetPercent(HealthPercent);
}
