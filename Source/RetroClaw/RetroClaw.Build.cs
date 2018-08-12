// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RetroClaw : ModuleRules
{
	public RetroClaw(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Paper2D" });
	}
}
