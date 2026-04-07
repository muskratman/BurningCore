// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CookieBrosPlatformer : ModuleRules
{
	public CookieBrosPlatformer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"AIModule",
				"EnhancedInput",
				"GameplayAbilities",
				"GameplayStateTreeModule",
				"GameplayTags",
				"GameplayTasks",
				"InputCore",
				"Slate",
				"StateTreeModule",
				"UMG"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"SlateCore"
			}
		);
	}
}
