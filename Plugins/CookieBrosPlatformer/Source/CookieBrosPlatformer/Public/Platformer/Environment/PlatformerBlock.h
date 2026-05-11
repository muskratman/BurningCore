#pragma once

#include "CoreMinimal.h"
#include "Platformer/Environment/PlatformerBlockBase.h"
#include "PlatformerBlock.generated.h"

/**
 * Simple cubic block with adjustable size.
 */
UCLASS()
class COOKIEBROSPLATFORMER_API APlatformerBlock : public APlatformerBlockBase
{
	GENERATED_BODY()

public:
	APlatformerBlock();
};
