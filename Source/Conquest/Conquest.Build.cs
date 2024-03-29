// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Conquest : ModuleRules
{
    public Conquest(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(new string[]
        {
            "Conquest/Public"
        });

        PrivateIncludePaths.AddRange(new string[]
        {
            "Conquest/Private"
        });

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "UMG"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "AIModule",
            "Slate",
            "SlateCore"
        });

        DynamicallyLoadedModuleNames.AddRange(new string[]
        {
            "OnlineSubsystemNull"
        });

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
