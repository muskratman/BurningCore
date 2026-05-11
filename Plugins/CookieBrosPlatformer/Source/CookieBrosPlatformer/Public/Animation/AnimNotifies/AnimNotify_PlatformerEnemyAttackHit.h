#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_PlatformerEnemyAttackHit.generated.h"

/**
 * Calls the pending enemy attack hit from an attack montage.
 *
 * Keep damage and projectile spawning in the enemy C++ class; this notify only
 * marks the authored impact frame.
 */
UCLASS(meta=(DisplayName="Platformer Enemy Attack Hit"))
class COOKIEBROSPLATFORMER_API UAnimNotify_PlatformerEnemyAttackHit : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
};
