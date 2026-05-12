#include "Platformer/Environment/PlatformerStairs.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"

APlatformerStairs::APlatformerStairs()
{
	PaletteIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/CookieBrosPlatformer/Textures/PlatformerStairs.PlatformerStairs")));
	BlockSize = FVector(100.0f, 500.0f, 100.0f);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> StairsMesh(TEXT("/CookieBrosPlatformer/Meshes/SM_Platform_Stairs.SM_Platform_Stairs"));
	if (StairsMesh.Succeeded())
	{
		BlockMesh->SetStaticMesh(StairsMesh.Object);
	}
}
