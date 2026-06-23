// Copyright Fortitudo Studio. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class RANDEditorTarget : TargetRules
{
	public RANDEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("RAND");
	}
}
