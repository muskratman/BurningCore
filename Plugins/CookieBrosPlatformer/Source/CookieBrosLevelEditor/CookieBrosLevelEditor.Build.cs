// Copyright CookieBros. All Rights Reserved.

using UnrealBuildTool;

public class CookieBrosLevelEditor : ModuleRules
{
	public CookieBrosLevelEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"CookieBrosPlatformer",
				"Paper2D"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"InputCore",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"LevelEditor",
				"PropertyEditor",
				"ToolMenus"
			}
		);
	}
}
