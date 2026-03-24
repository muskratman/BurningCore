#include "AI/EnemyBase.h"
#include "GAS/Attributes/EnemyAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/StateTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UEnemyAttributeSet>(TEXT("AttributeSet"));

	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTree"));
	
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
	// Add senses
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.0f;
	SightConfig->LoseSightRadius = 2000.0f;
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;
	SightConfig->SetMaxAge(5.0f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	PerceptionComponent->ConfigureSense(*SightConfig);
	
	UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
	DamageConfig->SetMaxAge(5.0f);
	PerceptionComponent->ConfigureSense(*DamageConfig);

	PerceptionComponent->SetDominantSense(UAISense_Sight::StaticClass());
	
	// Default to facing movement block logic
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

UAbilitySystemComponent* AEnemyBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AEnemyBase::ApplyDamage(const FGameplayEffectSpecHandle& DamageSpec, const FHitResult& HitResult)
{
	if (!AbilitySystemComponent || !DamageSpec.IsValid() || !IsAlive()) return;
	
	// Calculate damage magnitude for stagger logic before applying
	float PreHealth = AttributeSet->GetHealth();
	
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());
	
	float PostHealth = AttributeSet->GetHealth();
	float ActualDamage = PreHealth - PostHealth;

	if (!IsAlive())
	{
		HandleDeath();
	}
	else if (ActualDamage >= StaggerThreshold)
	{
		FGameplayEventData EventData;
		EventData.EventMagnitude = ActualDamage;
		AbilitySystemComponent->HandleGameplayEvent(FGameplayTag::RequestGameplayTag("Event.Hit.Received"), &EventData);
	}
}

bool AEnemyBase::IsAlive() const
{
	return AttributeSet && AttributeSet->GetHealth() > 0.0f;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		InitializeFromArchetype(DefaultArchetype);
	}
}

void AEnemyBase::InitializeFromArchetype(const UEnemyArchetypeAsset* Archetype)
{
	if (!Archetype || !AbilitySystemComponent) return;
	
	// Set baseline attributes
	AttributeSet->SetMaxHealth(3.0f); // Read from Archetype
	AttributeSet->SetHealth(3.0f);
	
	// In full implementation, grant abilities from Archetype->Abilities array
}

void AEnemyBase::HandleDeath()
{
	// Drop items
	// Play death animation via GAS or directly
	// Disable collision
	// Remove from StateTree evaluation
	AbilitySystemComponent->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag("State.Dead"));
}
