#include "AI/BossBase.h"
#include "GAS/Attributes/EnemyAttributeSet.h"

ABossBase::ABossBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UEnemyAttributeSet>(APlatformerCombatCharacterBase::GetAttributeSetSubobjectName()))
{
}
