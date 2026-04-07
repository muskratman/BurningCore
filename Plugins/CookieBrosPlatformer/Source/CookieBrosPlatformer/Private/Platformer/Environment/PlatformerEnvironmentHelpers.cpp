#include "Platformer/Environment/PlatformerEnvironmentHelpers.h"

#include "AbilitySystemComponent.h"
#include "Components/SceneComponent.h"
#include "AbilitySystemInterface.h"
#include "GAS/Attributes/PlatformerCharacterAttributeSet.h"
#include "GameplayEffect.h"
#include "Interfaces/Damageable.h"

namespace PlatformerEnvironment
{
	void ApplyRelativeTransform(
		USceneComponent* Component,
		const FVector& BaseLocation,
		const FRotator& BaseRotation,
		const FVector& BaseScale,
		const FPlatformerComponentTransformOffset& TransformOffset)
	{
		if (Component == nullptr)
		{
			return;
		}

		const FVector FinalScale = BaseScale * TransformOffset.RelativeScale3D;
		const FQuat FinalRotation = BaseRotation.Quaternion() * TransformOffset.RelativeRotation.Quaternion();

		Component->SetRelativeLocation(BaseLocation + TransformOffset.RelativeLocation);
		Component->SetRelativeRotation(FinalRotation);
		Component->SetRelativeScale3D(FinalScale);
	}

	bool ApplyConfiguredDamage(AActor* SourceActor, AActor* TargetActor, TSubclassOf<UGameplayEffect> DamageEffectClass, float DirectDamage, const FHitResult& HitResult)
	{
		if (!TargetActor || TargetActor == SourceActor)
		{
			return false;
		}

		IAbilitySystemInterface* AbilityTarget = Cast<IAbilitySystemInterface>(TargetActor);
		if (!AbilityTarget)
		{
			return false;
		}

		UAbilitySystemComponent* TargetASC = AbilityTarget->GetAbilitySystemComponent();
		if (!TargetASC)
		{
			return false;
		}

		if (DamageEffectClass)
		{
			FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
			ContextHandle.AddInstigator(SourceActor, SourceActor);
			ContextHandle.AddHitResult(HitResult);

			FGameplayEffectSpecHandle DamageSpec = TargetASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, ContextHandle);
			if (DamageSpec.IsValid())
			{
				if (IDamageable* DamageableTarget = Cast<IDamageable>(TargetActor))
				{
					DamageableTarget->ApplyDamage(DamageSpec, HitResult);
				}
				else
				{
					TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());
				}

				return true;
			}
		}

		if (DirectDamage <= 0.0f)
		{
			return false;
		}

		const FGameplayAttribute HealthAttribute = UPlatformerCharacterAttributeSet::GetHealthAttribute();
		if (!TargetASC->HasAttributeSetForAttribute(HealthAttribute))
		{
			return false;
		}

		TargetASC->ApplyModToAttribute(HealthAttribute, EGameplayModOp::Additive, -DirectDamage);
		return true;
	}
}
