#include "AI/PlatformerEnemyBase.h"

#include "AbilitySystemComponent.h"
#include "Components/StateTreeComponent.h"
#include "Data/PlatformerEnemyArchetypeAsset.h"
#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"
#include "GAS/PlatformerGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"

APlatformerEnemyBase::APlatformerEnemyBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	if (AttributeSet)
	{
		const float DefaultMaxHealth = GetDefaultMaxHealth();
		AttributeSet->InitMaxHealth(DefaultMaxHealth);
		AttributeSet->InitHealth(DefaultMaxHealth);
	}

	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTree"));

	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.0f;
	SightConfig->LoseSightRadius = 2000.0f;
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;
	SightConfig->SetMaxAge(5.0f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	PerceptionComponent->ConfigureSense(*SightConfig);

	DamageSenseConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
	DamageSenseConfig->SetMaxAge(5.0f);
	PerceptionComponent->ConfigureSense(*DamageSenseConfig);
	PerceptionComponent->SetDominantSense(UAISense_Sight::StaticClass());
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &APlatformerEnemyBase::HandleTargetPerceptionUpdated);

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	UpdateHealthWidgetPlacement();
}

void APlatformerEnemyBase::InitializeFromArchetype(const UPlatformerEnemyArchetypeAsset* Archetype)
{
	if (!Archetype || !AbilitySystemComponent || !AttributeSet)
	{
		return;
	}

	const float ResolvedBaseHealth =
		(Archetype->BaseHealth <= 0.0f || FMath::IsNearlyEqual(Archetype->BaseHealth, 3.0f))
			? GetDefaultMaxHealth()
			: Archetype->BaseHealth;

	AttributeSet->SetMaxHealth(ResolvedBaseHealth);
	AttributeSet->SetHealth(ResolvedBaseHealth);
	AttributeSet->SetBaseDamage(Archetype->BaseDamage);
	AttributeSet->SetAttackSpeed(Archetype->AttackSpeed);
	AttributeSet->SetMeleeAttackDamage(Archetype->BaseDamage);
	AttributeSet->SetRangeBaseAttackDamage(Archetype->BaseDamage);
	AttributeSet->SetMeleeAttackDelay(Archetype->AttackSpeed > 0.0f ? 1.0f / Archetype->AttackSpeed : 0.0f);
	AttributeSet->SetRangeAttackDelay(Archetype->AttackSpeed > 0.0f ? 1.0f / Archetype->AttackSpeed : 0.0f);
	AttributeSet->SetMoveSpeed(Archetype->MoveSpeed);

	CombatEngageRange = FMath::Max(Archetype->CombatEngageRange, 0.0f);
	CombatLoseTargetRange = FMath::Max(Archetype->CombatLoseTargetRange, CombatEngageRange);
	CombatAttackRange = FMath::Max(Archetype->CombatAttackRange, 0.0f);
	AttackDamageEffectClass = Archetype->DamageEffectClass;

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = Archetype->MoveSpeed;
	}

	if (SightConfig)
	{
		SightConfig->SightRadius = CombatEngageRange;
		SightConfig->LoseSightRadius = CombatLoseTargetRange;
	}

	if (PerceptionComponent)
	{
		PerceptionComponent->RequestStimuliListenerUpdate();
	}

	ApplyArchetypeCombatData(Archetype);

	if (StateTreeComponent && Archetype->BehaviorTree)
	{
		StateTreeComponent->SetStateTree(Archetype->BehaviorTree);
		StateTreeComponent->StartLogic();
	}

	if (HasAuthority())
	{
		for (const TSubclassOf<UGameplayAbility>& AbilityClass : Archetype->Abilities)
		{
			if (AbilityClass)
			{
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
			}
		}
	}

	SyncCombatLifeStateFromAttributes();
	RefreshHealthWidget();
}

void APlatformerEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	InitializeFromArchetype(DefaultArchetype);
}

void APlatformerEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CurrentCombatTarget)
	{
		return;
	}

	if (!CurrentCombatTarget->IsAlive() || GetCombatDistanceToTarget(CurrentCombatTarget) > CombatLoseTargetRange)
	{
		SetCombatTarget(nullptr);
	}
}

bool APlatformerEnemyBase::TryAttackCurrentTarget()
{
	return TryAttackTarget(CurrentCombatTarget);
}

bool APlatformerEnemyBase::TryAttackTarget(APlatformerCombatCharacterBase* TargetActor)
{
	if (!CanAttackTarget(TargetActor))
	{
		return false;
	}

	if (!PerformAttack(TargetActor))
	{
		return false;
	}

	if (UWorld* World = GetWorld())
	{
		LastAttackWorldTime = World->GetTimeSeconds();
	}

	return true;
}

void APlatformerEnemyBase::SetCombatTarget(APlatformerCombatCharacterBase* NewTarget)
{
	if (NewTarget == this || (NewTarget && !NewTarget->IsAlive()))
	{
		NewTarget = nullptr;
	}

	if (CurrentCombatTarget == NewTarget)
	{
		return;
	}

	APlatformerCombatCharacterBase* PreviousTarget = CurrentCombatTarget;
	CurrentCombatTarget = NewTarget;
	OnCombatTargetChanged(PreviousTarget, CurrentCombatTarget);
}

void APlatformerEnemyBase::OnCombatDamageReceived(float DamageAmount, const FHitResult& HitResult, AActor* DamageInstigatorActor)
{
	Super::OnCombatDamageReceived(DamageAmount, HitResult, DamageInstigatorActor);

	if (APlatformerCombatCharacterBase* DamageInstigatorCharacter = Cast<APlatformerCombatCharacterBase>(DamageInstigatorActor))
	{
		SetCombatTarget(DamageInstigatorCharacter);
	}

	if (AbilitySystemComponent && DamageAmount >= StaggerThreshold)
	{
		FGameplayEventData EventData;
		EventData.EventMagnitude = DamageAmount;
		EventData.Instigator = DamageInstigatorActor;
		EventData.Target = this;
		AbilitySystemComponent->HandleGameplayEvent(PlatformerGameplayTags::Event_Combat_HitReceived, &EventData);
	}
}

void APlatformerEnemyBase::OnCombatDeath(AActor* DamageInstigatorActor)
{
	Super::OnCombatDeath(DamageInstigatorActor);

	SetCombatTarget(nullptr);

	if (StateTreeComponent)
	{
		StateTreeComponent->StopLogic(TEXT("Combat death"));
	}

	if (PerceptionComponent)
	{
		PerceptionComponent->SetComponentTickEnabled(false);
	}

	if (HasAuthority())
	{
		Destroy();
	}
}

float APlatformerEnemyBase::GetDefaultMaxHealth() const
{
	return 100.0f;
}

float APlatformerEnemyBase::GetAttackRange() const
{
	return CombatAttackRange;
}

float APlatformerEnemyBase::GetAttackCooldown() const
{
	if (!AttributeSet)
	{
		return 0.0f;
	}

	const float AttackSpeed = AttributeSet->GetAttackSpeed();
	return AttackSpeed > 0.0f ? 1.0f / AttackSpeed : 0.0f;
}

float APlatformerEnemyBase::GetAttackDamageAmount() const
{
	if (!AttributeSet)
	{
		return 0.0f;
	}

	return FMath::Max(AttributeSet->GetBaseDamage(), 0.0f);
}

float APlatformerEnemyBase::GetProjectileMaxDistance() const
{
	return ProjectileMaxDistance;
}

float APlatformerEnemyBase::GetHealthWidgetVerticalPadding() const
{
	return PlatformerHealthWidgetVerticalPadding;
}

bool APlatformerEnemyBase::CanAttackTarget(const APlatformerCombatCharacterBase* TargetActor) const
{
	if (!TargetActor || !TargetActor->IsAlive() || !IsAlive())
	{
		return false;
	}

	if (GetCombatDistanceToTarget(TargetActor) > GetAttackRange())
	{
		return false;
	}

	if (const UWorld* World = GetWorld())
	{
		const float AttackCooldown = GetAttackCooldown();
		if (AttackCooldown > 0.0f && World->GetTimeSeconds() - LastAttackWorldTime < AttackCooldown)
		{
			return false;
		}
	}

	return true;
}

bool APlatformerEnemyBase::PerformAttack(APlatformerCombatCharacterBase* TargetActor)
{
	return false;
}

void APlatformerEnemyBase::ApplyArchetypeCombatData(const UPlatformerEnemyArchetypeAsset* Archetype)
{
}

void APlatformerEnemyBase::OnCombatTargetChanged(APlatformerCombatCharacterBase* PreviousTarget, APlatformerCombatCharacterBase* NewTarget)
{
	BP_OnCombatTargetChanged(PreviousTarget, NewTarget);
}

float APlatformerEnemyBase::GetCombatDistanceToTarget(const APlatformerCombatCharacterBase* TargetActor) const
{
	if (!TargetActor)
	{
		return TNumericLimits<float>::Max();
	}

	const FVector SourceLocation = GetActorLocation();
	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector FlattenedSourceLocation(SourceLocation.X, 0.0f, SourceLocation.Z);
	const FVector FlattenedTargetLocation(TargetLocation.X, 0.0f, TargetLocation.Z);
	return FVector::Dist(FlattenedSourceLocation, FlattenedTargetLocation);
}

void APlatformerEnemyBase::HandleTargetPerceptionUpdated(AActor* UpdatedActor, FAIStimulus Stimulus)
{
	APlatformerCombatCharacterBase* SensedCombatTarget = Cast<APlatformerCombatCharacterBase>(UpdatedActor);
	if (!SensedCombatTarget || SensedCombatTarget == this)
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		if (GetCombatDistanceToTarget(SensedCombatTarget) <= CombatEngageRange)
		{
			SetCombatTarget(SensedCombatTarget);
		}
	}
	else if (CurrentCombatTarget == SensedCombatTarget)
	{
		SetCombatTarget(nullptr);
	}
}
