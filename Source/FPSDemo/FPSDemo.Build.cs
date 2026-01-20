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


        string ProtobufBasePath = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty");
        PublicIncludePaths.Add(
                Path.Combine(ProtobufBasePath, "Protobuf", "include")
            );
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {

            PublicAdditionalLibraries.Add(
                Path.Combine(ProtobufBasePath, "Protobuf", "lib", "Win64", "libprotobuf.lib")
            );
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            PublicAdditionalLibraries.Add(
                Path.Combine(ProtobufBasePath, "Protobuf", "lib", "Linux", "libprotobuf.a")
            );
        }

    }
}
