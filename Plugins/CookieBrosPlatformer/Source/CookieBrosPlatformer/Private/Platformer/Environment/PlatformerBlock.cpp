#include "Platformer/Environment/PlatformerBlock.h"

APlatformerBlock::APlatformerBlock()
{
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerBlock.PlatformerBlock")));
}
