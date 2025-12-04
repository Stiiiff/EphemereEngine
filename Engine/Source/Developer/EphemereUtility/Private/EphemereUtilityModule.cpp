#include "Modules/ModuleManager.h"
#include "ShaderCore.h"
#include "Engine/EphemereSettings.h"

class FEphemereUtilityModule : public IModuleInterface
{
    public :
    virtual void StartupModule() override
    {
        FShaderCompilerEnvironment::ModifyCompilationEnvironmentDelegate.AddRaw(
            this, &FEphemereUtilityModule::InjectGlobalDefines);
    }

    void InjectGlobalDefines(FShaderCompilerEnvironment& OutEnv)
    {
        const UEphemereSettings* Settings = GetDefault<UEphemereSettings>
        OutEnv.SetDefine(TEXT("CURVE_ATLAS_MAXID"), 64)
    }
}