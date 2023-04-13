// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ActionGASProject : ModuleRules
{
	public ActionGASProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Sockets",
			"AIModule",
			"Networking",
			"GameplayAbilities",
			"GameplayTasks",
			"GameplayTags",
		});
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"HeadMountedDisplay",
			"UMG",
			"NavigationSystem", 
			"CommonInput", 
			"ChaosVehicles",
			"NetCore",
			"Niagara",
			"EnhancedInput"
		});
	}
}
