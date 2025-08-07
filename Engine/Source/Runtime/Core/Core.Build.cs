// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System;
using System.IO;

public class Core : ModuleRules
{
	public Core(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivatePCHHeaderFile = "Private/CorePrivatePCH.h";

		SharedPCHHeaderFile = "Public/CoreSharedPCH.h";

		PrivateDependencyModuleNames.Add("BuildSettings");

		PublicDependencyModuleNames.Add("TraceLog");
		PublicIncludePaths.Add("Runtime/TraceLog/Public");

		PrivateIncludePaths.AddRange(
			new string[] {
				"Developer/DerivedDataCache/Public",
				"Runtime/SynthBenchmark/Public",
				"Runtime/Core/Private",
				"Runtime/Core/Private/Misc",
                "Runtime/Core/Private/Internationalization",
				"Runtime/Core/Private/Internationalization/Cultures",
				"Runtime/Engine/Public",
			}
			);

		PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"TargetPlatform",
				"DerivedDataCache",
                "InputDevice",
                "Analytics",
				"RHI"
			}
			);

		if (Target.bBuildEditor == true)
		{
			DynamicallyLoadedModuleNames.Add("SourceCodeAccess");

			PrivateIncludePathModuleNames.Add("DirectoryWatcher");
			DynamicallyLoadedModuleNames.Add("DirectoryWatcher");
		}

		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Windows))
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"zlib");

			AddEngineThirdPartyPrivateStaticDependencies(Target, 
				"IntelTBB",
				"IntelVTune"
				);

			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"mimalloc");

			if(Target.WindowsPlatform.bUseBundledDbgHelp)
			{
				PublicDelayLoadDLLs.Add("DBGHELP.DLL");
				PrivateDefinitions.Add("USE_BUNDLED_DBGHELP=1");
				RuntimeDependencies.Add("$(EngineDir)/Binaries/ThirdParty/DbgHelp/dbghelp.dll");
			}
			else
			{
				PrivateDefinitions.Add("USE_BUNDLED_DBGHELP=0");
			}
			PrivateDefinitions.Add("YIELD_BETWEEN_TASKS=1");
		}
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Unix))
        {
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"zlib",
				"jemalloc"
                );

			// Core uses dlopen()
			PublicSystemLibraries.Add("dl");
        }

		if ( Target.bCompileICU == true )
        {
			AddEngineThirdPartyPrivateStaticDependencies(Target, "ICU");
        }
        PublicDefinitions.Add("UE_ENABLE_ICU=" + (Target.bCompileICU ? "1" : "0")); // Enable/disable (=1/=0) ICU usage in the codebase. NOTE: This flag is for use while integrating ICU and will be removed afterward.

        // If we're compiling with the engine, then add Core's engine dependencies
		if (Target.bCompileAgainstEngine == true)
		{
			if (!Target.bBuildRequiresCookedData)
			{
				DynamicallyLoadedModuleNames.AddRange(new string[] { "DerivedDataCache" });
			}
		}

		
		// On Windows platform, VSPerfExternalProfiler.cpp needs access to "VSPerf.h".  This header is included with Visual Studio, but it's not in a standard include path.
		if(Target.Platform.IsInGroup(UnrealPlatformGroup.Windows))
		{
			var VisualStudioVersionNumber = "11.0";
			var SubFolderName = "x64/PerfSDK";

			string PerfIncludeDirectory = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86), String.Format("Microsoft Visual Studio {0}/Team Tools/Performance Tools/{1}", VisualStudioVersionNumber, SubFolderName));

			if (File.Exists(Path.Combine(PerfIncludeDirectory, "VSPerf.h")))
			{
				PrivateIncludePaths.Add(PerfIncludeDirectory);
				PublicDefinitions.Add("WITH_VS_PERF_PROFILER=1");
			}
			else
			{
				PublicDefinitions.Add("WITH_VS_PERF_PROFILER=0");
			}
		}

		// Set a macro to allow FApp::GetBuildTargetType() to detect client targts
		if(Target.Type == TargetRules.TargetType.Client)
		{
			PrivateDefinitions.Add("IS_CLIENT_TARGET=1");
		}
		else
		{
			PrivateDefinitions.Add("IS_CLIENT_TARGET=0");
		}

		// Decide if validating memory allocator (aka MallocStomp) can be used on the current platform.
		// Run-time validation must be enabled through '-stompmalloc' command line argument.

		bool bWithMallocStomp = false;
        if (Target.Configuration != UnrealTargetConfiguration.Shipping)
        {
			if (Target.Platform == UnrealTargetPlatform.Linux
				|| Target.Platform == UnrealTargetPlatform.Win64)
			{
				bWithMallocStomp = true;
			}
        }

		// temporary thing.
		PrivateDefinitions.Add("PLATFORM_SUPPORTS_BINARYCONFIG=" + (SupportsBinaryConfig(Target) ? "1" : "0"));

        PublicDefinitions.Add("WITH_MALLOC_STOMP=" + (bWithMallocStomp ? "1" : "0"));

		PrivateDefinitions.Add("PLATFORM_COMPILER_OPTIMIZATION_LTCG=" + (Target.bAllowLTCG ? "1" : "0"));
		PrivateDefinitions.Add("PLATFORM_COMPILER_OPTIMIZATION_PG=" + (Target.bPGOOptimize ? "1" : "0"));
		PrivateDefinitions.Add("PLATFORM_COMPILER_OPTIMIZATION_PG_PROFILING=" + (Target.bPGOProfile ? "1" : "0"));

		UnsafeTypeCastWarningLevel = WarningLevel.Warning;
	}

	protected virtual bool SupportsBinaryConfig(ReadOnlyTargetRules Target)
	{
		return true;
	}
}
