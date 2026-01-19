// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class FPSDemo : ModuleRules
{
	public FPSDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", 
            "InputCore", "EnhancedInput", "PhysicsCore", "UMG", "Niagara", 
            "WebSockets", 
            "OnlineSubsystem",
            "OnlineSubsystemUtils" });

		PrivateDependencyModuleNames.AddRange(new string[] {
             "Slate",
			 "SlateCore"
        });

        PublicIncludePaths.AddRange(new[]
       {
            Path.Combine(ModuleDirectory, "Proto")
        });


        PublicIncludePaths.Add(
            Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "Protobuf", "include")
        );

        PublicAdditionalLibraries.Add(
            Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "Protobuf", "lib", "libprotobuf.lib")
        );
    }
}
