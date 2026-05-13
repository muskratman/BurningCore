#include "Systems/DragonDashEnemyTargetComponent.h"

bool UDragonDashEnemyTargetComponent::TryGetBounceDirection(FVector& OutDirection) const
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
