#pragma once

#include "CoreMinimal.h"
#include "Projectiles/BaseProjectile.h"
#include "ShotProjectile.generated.h"

/**
 * AShotProjectile
 * Concrete child class used for BaseShot and ChargeShot.
 */
UCLASS()
class BURNINGCORE_API AShotProjectile : public ABaseProjectile
{
	GENERATED_BODY()
	
public:
	AShotProjectile();
};
