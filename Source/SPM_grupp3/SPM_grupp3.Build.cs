// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;

public class SPM_grupp3 : ModuleRules
{
	public SPM_grupp3(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "NavigationSystem", "CinematicCamera", "AIModule", "FMODStudio" });

		// Slate/SlateCore needed for FReply, FSlateBrush in DrawingPuzzleWidget. //Izzy lagt till för ritpussel
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// FMOD's Blueprint statics (UFMODBlueprintStatics, UFMODBus) live in the plugin's private
		// "Classes" folder, which UE5.7 no longer auto-exposes to other modules. Add it so C++ can
		// call BusSetVolume directly (used by ULittleLost_GameInstance::ApplyMasterVolume).
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "..", "..", "Plugins", "FMODStudio", "Source", "FMODStudio", "Classes"));
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
