#include "Systems/DragonDashBounceSurfaceComponent.h"

bool UDragonDashBounceSurfaceComponent::TryGetBounceDirection(FVector& OutDirection) const
{
	if (!bOverrideBounceDirection)
	{
		return false;
	}

	const FVector SafeDirection = BounceDirection.GetSafeNormal();
	if (SafeDirection.IsNearlyZero())
	{
		return false;
	}

	OutDirection = SafeDirection;
	return true;
}
