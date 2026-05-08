#include "DMToolBoxEditor.h"

#include "Tool/DMToolBoxEditorMenu.h"

#define LOCTEXT_NAMESPACE "FDMToolBoxEditorModule"

void FDMToolBoxEditorModule::StartupModule()
{
	EditorMenu = MakeUnique<FDMToolBoxEditorMenu>();
	EditorMenu->Register();
}

void FDMToolBoxEditorModule::ShutdownModule()
{
	if (EditorMenu)
	{
		EditorMenu->Unregister();
		EditorMenu.Reset();
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDMToolBoxEditorModule, DMToolBoxEditor)
