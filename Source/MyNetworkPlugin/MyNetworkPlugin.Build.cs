// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MyNetworkPlugin : ModuleRules
{
	public MyNetworkPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput"
		, "OnlineSubsystemSteam", "OnlineSubsystem"});
	}
}
