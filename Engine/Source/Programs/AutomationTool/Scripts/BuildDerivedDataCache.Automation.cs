// Copyright Epic Games, Inc. All Rights Reserved.

using AutomationTool;
using UnrealBuildTool;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using Tools.DotNETCommon;

public class BuildDerivedDataCache : BuildCommand
{
	public override void ExecuteBuild()
	{
		// Get the list of platform names
		string TempDir = ParseParamValue("TempDir");
		UnrealTargetPlatform HostPlatform = BuildHostPlatform.Current.Platform;
		string TargetPlatforms = ParseParamValue("TargetPlatforms");
		string SavedDir = ParseParamValue("SavedDir");
		string BackendName = ParseParamValue("BackendName", "CreateInstalledEnginePak");
		string RelativePakPath = ParseParamValue("RelativePakPath", "Engine/DerivedDataCache/Compressed.ddp");
		bool bSkipEngine = ParseParam("SkipEngine");
		
		// Get paths to everything within the temporary directory
		string EditorExe = CommandUtils.GetEditorCommandletExe(TempDir, HostPlatform);
		string OutputPakFile = CommandUtils.CombinePaths(TempDir, RelativePakPath);
		string OutputCsvFile = Path.ChangeExtension(OutputPakFile, ".csv");


		List<string> ProjectPakFiles = new List<string>();
		// loop through all the projects first and bail out if one of them doesn't exist.

		// Generate DDC for the editor, and merge all the other PAK files in
		CommandUtils.LogInformation("Generating DDC data for engine content on {0}", TargetPlatforms);
		CommandUtils.DDCCommandlet(null, EditorExe, null, TargetPlatforms, String.Format("-fill -DDC={0}", bSkipEngine? " -projectonly" : ""));

		string SavedPakFile = CommandUtils.CombinePaths(SavedDir, RelativePakPath);
		CommandUtils.CopyFile(OutputPakFile, SavedPakFile);
	}
}

