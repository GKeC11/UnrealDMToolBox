// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DMToolBox : ModuleRules
{
	public DMToolBox(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				// Core framework
				"Core",
				"CoreUObject",
				"Engine",
				"DeveloperSettings",

				// UI framework
				"CommonUI",
				"UMG",

				// Gameplay framework
				"EnhancedInput",
				"GameplayTags",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// Asset and editor-adjacent runtime helpers
				"AssetRegistry",

				// Low-level UI implementation
				"Slate",
				"SlateCore",

				// Puerts scripting
				"Puerts",
				"JsEnv",

				// Gameplay messaging
				"GameplayMessageRuntime",

				// Server communication
				"Json",
				"JsonUtilities",
				"WebSockets",
			}
		);
	}
}
