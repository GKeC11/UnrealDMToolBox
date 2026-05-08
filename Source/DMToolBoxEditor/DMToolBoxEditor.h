#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FDMToolBoxEditorMenu;

class FDMToolBoxEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
	TUniquePtr<FDMToolBoxEditorMenu> EditorMenu;
};
