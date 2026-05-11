#include "Animation/PlatformerAnimGameplayTags.h"

namespace PlatformerAnimGameplayTags
{
	// Movement
	UE_DEFINE_GAMEPLAY_TAG(Anim_Movement_Jump, "Anim.Movement.Jump");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Movement_Dash, "Anim.Movement.Dash");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Movement_Crouch, "Anim.Movement.Crouch");

	// Combat
	UE_DEFINE_GAMEPLAY_TAG(Anim_Combat_MeleeHit, "Anim.Combat.MeleeHit");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Combat_MeleeChargeLoop, "Anim.Combat.MeleeChargeLoop");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Combat_RangedShot, "Anim.Combat.RangedShot");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Combat_RangedChargeLoop, "Anim.Combat.RangedChargeLoop");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Combat_HitReaction, "Anim.Combat.HitReaction");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Combat_Death, "Anim.Combat.Death");
}
