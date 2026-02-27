// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Senrenbanka : ModuleRules
{
	public Senrenbanka(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"HTTP",
			"Json",
			"JsonUtilities"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Senrenbanka",
			"Senrenbanka/Variant_Platforming",
			"Senrenbanka/Variant_Platforming/Animation",
			"Senrenbanka/Variant_Combat",
			"Senrenbanka/Variant_Combat/AI",
			"Senrenbanka/Variant_Combat/Animation",
			"Senrenbanka/Variant_Combat/Gameplay",
			"Senrenbanka/Variant_Combat/Interfaces",
			"Senrenbanka/Variant_Combat/UI",
			"Senrenbanka/Variant_SideScrolling",
			"Senrenbanka/Variant_SideScrolling/AI",
			"Senrenbanka/Variant_SideScrolling/Gameplay",
			"Senrenbanka/Variant_SideScrolling/Interfaces",
			"Senrenbanka/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
