#include "Animation/AnimNotifies/AnimNotify_PlatformerEnemyAttackHit.h"

#include "AI/PlatformerEnemyBase.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_PlatformerEnemyAttackHit::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	APlatformerEnemyBase* Enemy = MeshComp ? Cast<APlatformerEnemyBase>(MeshComp->GetOwner()) : nullptr;
	if (Enemy)
	{
		Enemy->ApplyPendingAttackHit();
	}
}

FString UAnimNotify_PlatformerEnemyAttackHit::GetNotifyName_Implementation() const
{
	return TEXT("Enemy Attack Hit");
}
