// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CommonMover : ModuleRules
{
	public CommonMover(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new []
		{ 
			"Core",
			"GameplayTags",
			"Mover",
			"EnhancedInput"
		});
			
		
		PrivateDependencyModuleNames.AddRange(new []
		{ 
			"CoreUObject", 
			"Engine", 
		});
	}
}
