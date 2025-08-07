// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AudioCaptureEditor : ModuleRules
{
	public AudioCaptureEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"UnrealEd",
				"SequenceRecorder",
				"AudioEditor",
				"AudioCapture",
				"AudioCaptureCore",
				"AudioCaptureRtAudio"
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
				"AudioCaptureEditor/Private",
				"AudioCapture/Private",
				"../../../../Source/Runtime/AudioCaptureImplementations/AudioCaptureRtAudio/Private" // This is required to include RtAudio.h in AudioRecordingManager.h.
			}
		);
	}
}
