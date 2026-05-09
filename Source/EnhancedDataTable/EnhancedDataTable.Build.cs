// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class EnhancedDataTable : ModuleRules
{
	public EnhancedDataTable(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine"
			}
		);

		if (Target.bBuildEditor)
		{
			bEnableExceptions = true;

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"ApplicationCore",
					"AssetDefinition",
					"DesktopPlatform",
					"InputCore",
					"PropertyEditor",
					"Slate",
					"SlateCore",
					"UnrealEd"
				}
			);

			string PluginDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
			string OpenXlsxDirectory = Path.Combine(PluginDirectory, "ThirdParty", "OpenXLSX");

			PrivateDefinitions.Add("OPENXLSX_STATIC_DEFINE=1");

			PrivateIncludePaths.AddRange(
				new string[]
				{
					Path.Combine(OpenXlsxDirectory, "OpenXLSX"),
					Path.Combine(OpenXlsxDirectory, "OpenXLSX", "headers"),
					Path.Combine(OpenXlsxDirectory, "external", "miniz"),
					Path.Combine(OpenXlsxDirectory, "external", "pugixml", "src")
				}
			);
		}
	}
}
