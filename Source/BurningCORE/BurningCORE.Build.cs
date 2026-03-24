// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BurningCORE : ModuleRules
{
	public BurningCORE(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"BurningCORE",
			"BurningCORE/Variant_Platforming",
			"BurningCORE/Variant_Platforming/Animation",
			"BurningCORE/Variant_Combat",
			"BurningCORE/Variant_Combat/AI",
			"BurningCORE/Variant_Combat/Animation",
			"BurningCORE/Variant_Combat/Gameplay",
			"BurningCORE/Variant_Combat/Interfaces",
			"BurningCORE/Variant_Combat/UI",
			"BurningCORE/Variant_SideScrolling",
			"BurningCORE/Variant_SideScrolling/AI",
			"BurningCORE/Variant_SideScrolling/Gameplay",
			"BurningCORE/Variant_SideScrolling/Interfaces",
			"BurningCORE/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
