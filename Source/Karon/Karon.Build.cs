// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Karon : ModuleRules
{
	public Karon(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine",
			"InputCore", "EnhancedInput",
			"GameplayAbilities", "GameplayTags", "GameplayTasks",
			"AnimGraphRuntime","MotionTrajectory", "PoseSearch", "MotionWarping",
			// CommonUI
			"CommonUI", "CommonInput",
			// StructUtils (FInstancedStruct)
			"StructUtils",
			// GMRouter (GameplayMessage Router subsystem)
			"GMRouter",
			// UDeveloperSettings (Project Settings 통합)
			"DeveloperSettings",
			"Niagara",
			"NiagaraAnimNotifies",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate", "SlateCore",
			"UMG",
			"NavigationSystem", "SQLiteCore", 
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "AIModule", });
	
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true

		PublicIncludePaths.AddRange(new string[] { "Karon" });
	}
}
