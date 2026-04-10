#include "AI/EnemyFlying.h"
#include "GAS/Attributes/EnemyAttributeSet.h"

AEnemyFlying::AEnemyFlying(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UEnemyAttributeSet>(APlatformerCombatCharacterBase::GetAttributeSetSubobjectName()))
{
}
