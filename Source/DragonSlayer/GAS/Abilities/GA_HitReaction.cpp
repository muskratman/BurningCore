#include "GAS/Abilities/GA_HitReaction.h"

UGA_HitReaction::UGA_HitReaction()
{
}

UAnimMontage* UGA_HitReaction::GetHitReactionMontage(const FGameplayAbilityActorInfo* ActorInfo) const
{
	return HitReactMontage;
}
