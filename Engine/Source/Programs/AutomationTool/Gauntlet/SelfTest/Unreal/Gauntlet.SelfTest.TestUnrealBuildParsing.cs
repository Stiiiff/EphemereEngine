// Copyright Epic Games, Inc. All Rights Reserved.

using Gauntlet;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnrealBuildTool;

namespace Gauntlet.SelfTest
{
	/// <summary>
	/// This test validates that our helpers can correctly extract platform/configuration information from
	/// filenames. This is important when discovering builds
	/// </summary>
	[TestGroup("Framework")]
	class TestUnrealBuildParsing : TestUnrealBase
	{

		struct BuildData
		{
			public string ProjectName;
			public string FileName;
			public UnrealTargetConfiguration ExpectedConfiguration;
			public UnrealTargetPlatform ExpectedPlatform;
			public UnrealTargetRole ExpectedRole;
			public bool ContentProject;

			public BuildData(string InProjectName, string InFilename, UnrealTargetConfiguration InConfiguration, UnrealTargetPlatform InPlatform, UnrealTargetRole InRole, bool InContentProject=false)
			{
				ProjectName = InProjectName;
				FileName = InFilename;
				ExpectedConfiguration = InConfiguration;
				ExpectedPlatform = InPlatform;
				ExpectedRole = InRole;
				ContentProject = InContentProject;
			}
		};

		public override void TickTest()
		{

			BuildData[] TestData =
			{
				// Test a content project
				new BuildData("ElementalDemo", "UE4Game.exe", UnrealTargetConfiguration.Development, UnrealTargetPlatform.Win64, UnrealTargetRole.Client, true ),
				new BuildData("ElementalDemo", "UE4Game-Win64-Test.exe", UnrealTargetConfiguration.Test, UnrealTargetPlatform.Win64, UnrealTargetRole.Client, true ),
				new BuildData("ElementalDemo", "UE4Game-Win64-Shippinge.exe", UnrealTargetConfiguration.Shipping, UnrealTargetPlatform.Win64, UnrealTargetRole.Client, true ),

				// Test a regular monolithic project
				new BuildData("ActionRPG", "ActionRPG.exe", UnrealTargetConfiguration.Development, UnrealTargetPlatform.Win64, UnrealTargetRole.Client ),
				new BuildData("ActionRPG", "ActionRPG-Win64-Test.exe", UnrealTargetConfiguration.Test, UnrealTargetPlatform.Win64, UnrealTargetRole.Client ),
				new BuildData("ActionRPG", "ActionRPG-Win64-Shipping.exe", UnrealTargetConfiguration.Shipping, UnrealTargetPlatform.Win64, UnrealTargetRole.Client ),
				
				// Test a project like ForniteGame that emits dedicated Client executables
				new BuildData("FortniteGame", "FortniteClient.exe", UnrealTargetConfiguration.Development, UnrealTargetPlatform.Win64, UnrealTargetRole.Client ),
				new BuildData("FortniteGame", "FortniteClient-Win64-Test.exe", UnrealTargetConfiguration.Test, UnrealTargetPlatform.Win64, UnrealTargetRole.Client ),
				new BuildData("FortniteGame", "FortniteClient-Win64-Shipping.exe", UnrealTargetConfiguration.Shipping, UnrealTargetPlatform.Win64, UnrealTargetRole.Client ),

				// Test a project like ForniteGame that emits dedicated Server executables
				new BuildData("FortniteGame", "FortniteServer.exe", UnrealTargetConfiguration.Development, UnrealTargetPlatform.Win64, UnrealTargetRole.Server ),
				new BuildData("FortniteGame", "FortniteServer-Win64-Test.exe", UnrealTargetConfiguration.Test, UnrealTargetPlatform.Win64, UnrealTargetRole.Server ),
				new BuildData("FortniteGame", "FortniteServer-Win64-Shipping.exe", UnrealTargetConfiguration.Shipping, UnrealTargetPlatform.Win64, UnrealTargetRole.Server ),
				new BuildData("FortniteGame", "FortniteServer", UnrealTargetConfiguration.Development, UnrealTargetPlatform.Linux, UnrealTargetRole.Server ),
				new BuildData("FortniteGame", "FortniteServer-Linux-Test", UnrealTargetConfiguration.Test, UnrealTargetPlatform.Linux, UnrealTargetRole.Server ),
				new BuildData("FortniteGame", "FortniteServer-Linux-Shipping", UnrealTargetConfiguration.Shipping, UnrealTargetPlatform.Linux, UnrealTargetRole.Server ),
			};

			foreach (BuildData Data in TestData)
			{
				string Executable = Data.FileName;

				var FoundConfig = UnrealHelpers.GetConfigurationFromExecutableName(Data.ProjectName, Data.FileName);
				var FoundRole = UnrealHelpers.GetRoleFromExecutableName(Data.ProjectName, Data.FileName);
				var CalculatedName = UnrealHelpers.GetExecutableName(Data.ProjectName, Data.ExpectedPlatform, Data.ExpectedConfiguration, Data.ExpectedRole, Path.GetExtension(Data.FileName));

				CheckResult(FoundConfig == Data.ExpectedConfiguration, "Parsed configuration was wrong for {0}. Detected {1}, Expected {2}", Data.FileName, FoundConfig, Data.ExpectedConfiguration);
				CheckResult(FoundRole == Data.ExpectedRole, "Parsed role was wrong for {0}. Detected {1}, Expected {2}", Data.FileName, FoundRole, Data.ExpectedRole);
				CheckResult(Data.ContentProject || CalculatedName.Equals(Data.FileName, StringComparison.OrdinalIgnoreCase), "Generated name from platform/config/was wrong. Calcualted {0}, Expected {1}", CalculatedName, Data.FileName);
			}


			MarkComplete();
		}

	}
}