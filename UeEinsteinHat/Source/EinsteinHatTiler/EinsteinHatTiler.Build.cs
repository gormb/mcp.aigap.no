using UnrealBuildTool;

public class EinsteinHatTiler : ModuleRules
{
	public EinsteinHatTiler(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"ProceduralMeshComponent"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}
