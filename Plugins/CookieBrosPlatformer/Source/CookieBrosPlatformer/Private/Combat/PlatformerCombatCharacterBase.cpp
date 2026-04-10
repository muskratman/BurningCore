#include "Combat/PlatformerCombatCharacterBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"
#include "GAS/Effects/PlatformerDamageGameplayEffect.h"
#include "GAS/PlatformerGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/PlatformerHealthWidget.h"

APlatformerCombatCharacterBase::APlatformerCombatCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UPlatformerCharacterAttributeSet>(GetAttributeSetSubobjectName());

	HealthWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidgetComponent"));
	HealthWidgetComponent->SetupAttachment(RootComponent);
	HealthWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthWidgetComponent->SetDrawAtDesiredSize(false);
	HealthWidgetComponent->SetDrawSize(HealthWidgetDrawSize);
	HealthWidgetComponent->SetPivot(FVector2D(0.5f, 1.0f));
	HealthWidgetComponent->SetTwoSided(true);
	HealthWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthWidgetComponent->SetGenerateOverlapEvents(false);
	UpdateHealthWidgetPlacement();
}

UAbilitySystemComponent* APlatformerCombatCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

FName APlatformerCombatCharacterBase::GetAttributeSetSubobjectName()
{
	static const FName AttributeSetSubobjectName(TEXT("AttributeSet"));
	return AttributeSetSubobjectName;
}

void APlatformerCombatCharacterBase::ApplyDamage(const FGameplayEffectSpecHandle& DamageSpec, const FHitResult& HitResult)
{
	if (!AbilitySystemComponent || !DamageSpec.IsValid() || !IsAlive())
	{
		return;
	}

	const float PreviousHealth = AttributeSet ? AttributeSet->GetHealth() : 0.0f;

	AActor* DamageInstigatorActor = nullptr;
	if (DamageSpec.Data.IsValid())
	{
		const FGameplayEffectContextHandle& EffectContext = DamageSpec.Data->GetContext();
		DamageInstigatorActor = EffectContext.GetOriginalInstigator();
		if (!DamageInstigatorActor)
		{
			DamageInstigatorActor = EffectContext.GetInstigator();
		}
	}

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());

	const float CurrentHealth = AttributeSet ? AttributeSet->GetHealth() : 0.0f;
	const float ActualDamage = FMath::Max(PreviousHealth - CurrentHealth, 0.0f);

	if (ActualDamage > 0.0f)
	{
		LastDamageInstigatorActor = DamageInstigatorActor;
		OnCombatDamageReceived(ActualDamage, HitResult, DamageInstigatorActor);
	}

	SyncCombatLifeStateFromAttributes();
}

bool APlatformerCombatCharacterBase::IsAlive() const
{
	return AttributeSet && AttributeSet->GetHealth() > 0.0f;
}

FGameplayEffectSpecHandle APlatformerCombatCharacterBase::MakeCombatDamageEffectSpec(
	float DamageAmount,
	const FHitResult& HitResult,
	TSubclassOf<UGameplayEffect> DamageEffectClass,
	float EffectLevel) const
{
	if (!AbilitySystemComponent || DamageAmount <= 0.0f)
	{
		return FGameplayEffectSpecHandle();
	}

	TSubclassOf<UGameplayEffect> ResolvedDamageEffectClass = DamageEffectClass;
	if (!ResolvedDamageEffectClass)
	{
		ResolvedDamageEffectClass = UPlatformerDamageGameplayEffect::StaticClass();
	}

	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddInstigator(const_cast<APlatformerCombatCharacterBase*>(this), const_cast<APlatformerCombatCharacterBase*>(this));
	ContextHandle.AddSourceObject(const_cast<APlatformerCombatCharacterBase*>(this));
	ContextHandle.AddHitResult(HitResult);

	FGameplayEffectSpecHandle DamageSpec = AbilitySystemComponent->MakeOutgoingSpec(ResolvedDamageEffectClass, EffectLevel, ContextHandle);
	if (DamageSpec.IsValid())
	{
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
			DamageSpec,
			PlatformerGameplayTags::Data_Combat_Damage,
			DamageAmount);
	}

	return DamageSpec;
}

bool APlatformerCombatCharacterBase::ApplyCombatDamageToActor(
	AActor* TargetActor,
	float DamageAmount,
	const FHitResult& HitResult,
	TSubclassOf<UGameplayEffect> DamageEffectClass,
	float EffectLevel) const
{
	if (!TargetActor || TargetActor == this)
	{
		return false;
	}

	const FGameplayEffectSpecHandle DamageSpec = MakeCombatDamageEffectSpec(DamageAmount, HitResult, DamageEffectClass, EffectLevel);
	if (!DamageSpec.IsValid())
	{
		return false;
	}

	if (IDamageable* DamageableTarget = Cast<IDamageable>(TargetActor))
	{
		DamageableTarget->ApplyDamage(DamageSpec, HitResult);
		return true;
	}

	if (IAbilitySystemInterface* AbilityTarget = Cast<IAbilitySystemInterface>(TargetActor))
	{
		if (UAbilitySystemComponent* TargetASC = AbilityTarget->GetAbilitySystemComponent())
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());
			return true;
		}
	}

	return false;
}

void APlatformerCombatCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		HealthChangedDelegateHandle =
			AbilitySystemComponent
				->GetGameplayAttributeValueChangeDelegate(UPlatformerCharacterAttributeSet::GetHealthAttribute())
				.AddUObject(this, &APlatformerCombatCharacterBase::HandleHealthChanged);

		MaxHealthChangedDelegateHandle =
			AbilitySystemComponent
				->GetGameplayAttributeValueChangeDelegate(UPlatformerCharacterAttributeSet::GetMaxHealthAttribute())
				.AddUObject(this, &APlatformerCombatCharacterBase::HandleMaxHealthChanged);
	}

	UpdateHealthWidgetPlacement();
	RefreshHealthWidget();
	SyncCombatLifeStateFromAttributes();
}

void APlatformerCombatCharacterBase::OnCombatDamageReceived(float DamageAmount, const FHitResult& HitResult, AActor* DamageInstigatorActor)
{
	BP_OnCombatDamageReceived(
		DamageAmount,
		HitResult.ImpactPoint.IsNearlyZero() ? HitResult.Location : HitResult.ImpactPoint,
		DamageInstigatorActor);
}

void APlatformerCombatCharacterBase::OnCombatDeath(AActor* DamageInstigatorActor)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(PlatformerGameplayTags::State_Combat_Dead);

		FGameplayEventData EventData;
		EventData.Instigator = DamageInstigatorActor;
		EventData.Target = this;
		AbilitySystemComponent->HandleGameplayEvent(PlatformerGameplayTags::Event_Combat_Death, &EventData);
	}

	if (bDisableMovementOnDeath)
	{
		if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
		{
			MovementComponent->DisableMovement();
			MovementComponent->StopMovementImmediately();
		}
	}

	if (DeathLifeSpan > 0.0f)
	{
		SetLifeSpan(DeathLifeSpan);
	}

	BP_OnCombatDeath(DamageInstigatorActor);
}

void APlatformerCombatCharacterBase::OnCombatRevived()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(PlatformerGameplayTags::State_Combat_Dead);
	}
}

float APlatformerCombatCharacterBase::GetHealthWidgetVerticalPadding() const
{
	return 20.0f;
}

void APlatformerCombatCharacterBase::RefreshHealthWidget()
{
	if (!HealthWidgetComponent)
	{
		return;
	}

	if (!HealthWidgetClass)
	{
		HealthWidgetComponent->SetVisibility(false);
		return;
	}

	HealthWidgetComponent->SetVisibility(true);

	if (HealthWidgetComponent->GetWidgetClass() != HealthWidgetClass)
	{
		HealthWidgetComponent->SetWidgetClass(HealthWidgetClass);
	}

	HealthWidgetComponent->SetDrawSize(HealthWidgetDrawSize);
	HealthWidgetComponent->InitWidget();

	if (UPlatformerHealthWidget* HealthWidget = Cast<UPlatformerHealthWidget>(HealthWidgetComponent->GetUserWidgetObject()))
	{
		const float CurrentHealth = AttributeSet ? AttributeSet->GetHealth() : 0.0f;
		const float MaxHealth = AttributeSet ? AttributeSet->GetMaxHealth() : 1.0f;
		HealthWidget->SetHealthValues(CurrentHealth, MaxHealth);
	}
}

void APlatformerCombatCharacterBase::UpdateHealthWidgetPlacement()
{
	if (!HealthWidgetComponent)
	{
		return;
	}

	float CapsuleHalfHeight = 0.0f;
	if (const UCapsuleComponent* CharacterCapsule = GetCapsuleComponent())
	{
		CapsuleHalfHeight = CharacterCapsule->GetScaledCapsuleHalfHeight();
	}

	HealthWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, CapsuleHalfHeight + GetHealthWidgetVerticalPadding()));
}

void APlatformerCombatCharacterBase::SyncCombatLifeStateFromAttributes()
{
	if (IsAlive())
	{
		if (bCombatDeathHandled)
		{
			bCombatDeathHandled = false;
			OnCombatRevived();
		}

		return;
	}

	if (!bCombatDeathHandled)
	{
		bCombatDeathHandled = true;
		OnCombatDeath(LastDamageInstigatorActor.Get());
	}
}

void APlatformerCombatCharacterBase::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue == Data.OldValue)
	{
		return;
	}

	RefreshHealthWidget();
	SyncCombatLifeStateFromAttributes();
}

void APlatformerCombatCharacterBase::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue == Data.OldValue)
	{
		return;
	}

	RefreshHealthWidget();
}
