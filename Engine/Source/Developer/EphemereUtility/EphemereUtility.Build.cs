using UnrealBuildTool;

public class EphemereUtilityModule : ModuleRules
{
    public EphemereUtilityModule(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "RenderCore", "RHI", "Projects"});
    }
}