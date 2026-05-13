#include "Input/PlatformerInputConfig.h"

#include "InputAction.h"

UInputAction* UPlatformerInputConfig::FindInputAction(const FGameplayTag& InputTag) const
{
	if (!InputTag.IsValid())
	{
		return nullptr;
	}

	for (const FPlatformerInputActionBinding& Binding : InputBindings)
	{
		if (Binding.InputTag == InputTag)
		{
			return Binding.InputAction;
		}
	}

	return nullptr;
}
