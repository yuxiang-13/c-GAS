// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ActionGASProject : ModuleRules
{
	public ActionGASProject(ReadOnlyTargetRules Target) : base(Target)
	{
		// PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		//
		// PublicDependencyModuleNames.AddRange(new string[]
		// {
		// 	"Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay",
		// 	"GameplayAbilities", 
		// });
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Sockets",
			"GameplayTasks",
			"AIModule",
			"GameplayAbilities",
			"Networking"
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
		});
	}
}
