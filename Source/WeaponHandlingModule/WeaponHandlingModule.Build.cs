// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class WeaponHandlingModule : ModuleRules
{
	public WeaponHandlingModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine"
		});

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"EnhancedInput"
			});

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			});
	}
}