#include "AI/EnemyRanged.h"
#include "GAS/Attributes/EnemyAttributeSet.h"

AEnemyRanged::AEnemyRanged(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UEnemyAttributeSet>(APlatformerCombatCharacterBase::GetAttributeSetSubobjectName()))
{
	// Setup specific ranged behaviors or components if needed
}
