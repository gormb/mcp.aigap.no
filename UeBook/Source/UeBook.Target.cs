using UnrealBuildTool;
using System.Collections.Generic;

public class UeBookTarget : TargetRules
{
	public UeBookTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("UeBook");
	}
}
