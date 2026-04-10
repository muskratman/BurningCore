#include "AI/PlatformerBossBase.h"

#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/PlatformerBossEncounterRegistry.h"
#include "Kismet/GameplayStatics.h"

APlatformerBossBase::APlatformerBossBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Bosses generally have much larger stagger thresholds or are immune to basic hit reactions.
	StaggerThreshold = 50.0f;

	if (AttributeSet)
	{
		const float DefaultMaxHealth = GetDefaultMaxHealth();
		AttributeSet->InitMaxHealth(DefaultMaxHealth);
		AttributeSet->InitHealth(DefaultMaxHealth);
	}

	UpdateHealthWidgetPlacement();
}

void APlatformerBossBase::BeginPlay()
{
	Super::BeginPlay();

	if (AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
		IsValid(GameMode) && GameMode->GetClass()->ImplementsInterface(UPlatformerBossEncounterRegistry::StaticClass()))
	{
		IPlatformerBossEncounterRegistry::Execute_RegisterBossEncounterActor(GameMode, this);
	}
}

void APlatformerBossBase::TransitionToPhase_Implementation(int32 NextPhase)
{
	CurrentPhase = NextPhase;
}

float APlatformerBossBase::GetDefaultMaxHealth() const
{
	return 300.0f;
}

float APlatformerBossBase::GetProjectileMaxDistance() const
{
	return BossProjectileMaxDistance;
}

float APlatformerBossBase::GetHealthWidgetVerticalPadding() const
{
	return BossHealthWidgetVerticalPadding;
}
