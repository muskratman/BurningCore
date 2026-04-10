 #pragma once

#include "CoreMinimal.h"
#include "Projectiles/Combat/CombatProjectile.h"
#include "DragonProjectile.generated.h"

class UNiagaraComponent;

/**
 * ADragonProjectile
 * Base class for all projectiles fired by the player.
 */
UCLASS(abstract)
class DRAGONSLAYER_API ADragonProjectile : public ACombatProjectile
{
	GENERATED_BODY()
	
public:	
	ADragonProjectile();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UNiagaraComponent> TrailVFX;
};
