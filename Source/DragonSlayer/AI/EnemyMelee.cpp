#include "AI/EnemyMelee.h"
#include "GAS/Attributes/EnemyAttributeSet.h"

AEnemyMelee::AEnemyMelee(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UEnemyAttributeSet>(APlatformerCombatCharacterBase::GetAttributeSetSubobjectName()))
{
	// Setup specific melee behaviors
}
