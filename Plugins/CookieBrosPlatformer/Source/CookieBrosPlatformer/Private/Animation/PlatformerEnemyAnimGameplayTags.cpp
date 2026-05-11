#include "Animation/PlatformerEnemyAnimGameplayTags.h"

namespace PlatformerEnemyAnimGameplayTags
{
	// Movement
	UE_DEFINE_GAMEPLAY_TAG(Anim_Enemy_Movement_Jump, "Anim.Enemy.Movement.Jump");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Enemy_Movement_Fall, "Anim.Enemy.Movement.Fall");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Enemy_Movement_Land, "Anim.Enemy.Movement.Land");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Enemy_Movement_Fly, "Anim.Enemy.Movement.Fly");

	// Combat
	UE_DEFINE_GAMEPLAY_TAG(Anim_Enemy_Combat_MeleeAttack, "Anim.Enemy.Combat.MeleeAttack");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Enemy_Combat_RangedAttack, "Anim.Enemy.Combat.RangedAttack");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Enemy_Combat_SpecialAttack, "Anim.Enemy.Combat.SpecialAttack");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Enemy_Combat_HitReaction, "Anim.Enemy.Combat.HitReaction");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Enemy_Combat_Death, "Anim.Enemy.Combat.Death");
}
