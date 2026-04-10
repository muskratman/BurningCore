#pragma once

#include "CoreMinimal.h"
#include "Character/PlatformerCharacterBase.h"
#include "DragonCharacter.generated.h"

class UDragonAttributeSet;
class UDragonFormComponent;
class UDragonOverdriveComponent;

/**
 * ADragonCharacter
 * Base class for the primary playable hero.
 */
UCLASS(abstract)
class DRAGONSLAYER_API ADragonCharacter : public APlatformerCharacterBase
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UDragonFormComponent> FormComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UDragonOverdriveComponent> OverdriveComponent;

public:
	ADragonCharacter(const FObjectInitializer& ObjectInitializer);
	UDragonAttributeSet* GetDragonAttributeSet() const;
	float GetProjectileMaxDistance() const { return ProjectileMaxDistance; }

public:
	FORCEINLINE UDragonFormComponent* GetFormComponent() const { return FormComponent; }
	FORCEINLINE UDragonOverdriveComponent* GetOverdriveComponent() const { return OverdriveComponent; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Projectile", meta=(ClampMin=0.0, Units="cm"))
	float ProjectileMaxDistance = 1500.0f;
};
