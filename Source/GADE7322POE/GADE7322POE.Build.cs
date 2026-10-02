// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GADE7322POE : ModuleRules
{
	public GADE7322POE(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });
		
		
		
	}
}
