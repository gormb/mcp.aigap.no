using UnrealBuildTool;
using System.Collections.Generic;

public class UeEinsteinHatTarget : TargetRules
{
	public UeEinsteinHatTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("EinsteinHatTiler");
	}
}
