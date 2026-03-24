#include "GAS/Abilities/GA_FormSwitch.h"
#include "Character/DragonCharacter.h"
#include "Character/DragonFormComponent.h"

UGA_FormSwitch::UGA_FormSwitch()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// SetAssetTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Form.Switch"))));
	
	// Add cooldown tag
	// SetActivationOwnedTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Action.FormSwitch"))));
}

void UGA_FormSwitch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ADragonCharacter* Character = Cast<ADragonCharacter>(ActorInfo->AvatarActor.Get());
	if (Character && Character->GetFormComponent())
	{
		if (TargetFormTag.IsValid())
		{
			Character->GetFormComponent()->SwitchForm(TargetFormTag);
		}
		else
		{
			// In a full implementation, you'd check ProgressionSubsystem for unlocked forms
			// and cycle through them. For now, simple toggle between Base and Fire:
			FGameplayTag BaseForm = FGameplayTag::RequestGameplayTag(FName("Form.Base"));
			FGameplayTag FireForm = FGameplayTag::RequestGameplayTag(FName("Form.Fire"));

			if (Character->GetFormComponent()->GetActiveFormTag() == BaseForm)
			{
				Character->GetFormComponent()->SwitchForm(FireForm);
			}
			else
			{
				Character->GetFormComponent()->SwitchForm(BaseForm);
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
