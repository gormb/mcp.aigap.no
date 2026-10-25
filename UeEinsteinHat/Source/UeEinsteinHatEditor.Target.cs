using UnrealBuildTool;
using System.Collections.Generic;

public class UeEinsteinHatEditorTarget : TargetRules
{
	public UeEinsteinHatEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("EinsteinHatTiler");
	}
}
