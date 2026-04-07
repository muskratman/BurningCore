#include "Platformer/Environment/PlatformerStairs.h"

#include "Engine/Texture2D.h"

APlatformerStairs::APlatformerStairs()
{
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerStairs.PlatformerStairs")));
	BlockSize = FVector(100.0f, 500.0f, 100.0f);
}
