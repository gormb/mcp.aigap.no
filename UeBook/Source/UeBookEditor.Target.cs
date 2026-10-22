using UnrealBuildTool;
using System.Collections.Generic;

public class UeBookEditorTarget : TargetRules
{
	public UeBookEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("UeBook");
	}
}
