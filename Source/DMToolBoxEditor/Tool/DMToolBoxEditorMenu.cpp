#include "DMToolBoxEditorMenu.h"

#include "DMGameplayTagAccessorGenerator.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LevelEditor.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FDMToolBoxEditorMenu"

void FDMToolBoxEditorMenu::Register()
{
	if (MenuBarExtender.IsValid())
	{
		return;
	}

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	MenuBarExtender = MakeShared<FExtender>();
	MenuBarExtender->AddMenuBarExtension(
		"Help",
		EExtensionHook::After,
		nullptr,
		FMenuBarExtensionDelegate::CreateRaw(this, &FDMToolBoxEditorMenu::AddMenuBarExtension));

	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuBarExtender);
}

void FDMToolBoxEditorMenu::Unregister()
{
	if (!MenuBarExtender.IsValid())
	{
		return;
	}

	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.GetMenuExtensibilityManager()->RemoveExtender(MenuBarExtender);
	}

	MenuBarExtender.Reset();
}

void FDMToolBoxEditorMenu::AddMenuBarExtension(FMenuBarBuilder& MenuBarBuilder)
{
	MenuBarBuilder.AddPullDownMenu(
		LOCTEXT("DMToolBoxMenuLabel", "DMToolBox"),
		LOCTEXT("DMToolBoxMenuToolTip", "Open DMToolBox editor tools."),
		FNewMenuDelegate::CreateRaw(this, &FDMToolBoxEditorMenu::FillPulldownMenu),
		"DMToolBox");
}

void FDMToolBoxEditorMenu::FillPulldownMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("DMToolBoxTools", LOCTEXT("DMToolBoxToolsSection", "Tools"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("GenerateGameplayTagsLabel", "Generate GameplayTags"),
			LOCTEXT("GenerateGameplayTagsToolTip", "Generate NOGameplayTag code from DataTables listed in GameplayTagTableList."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FDMToolBoxEditorMenu::GenerateGameplayTagAccessors)),
			NAME_None,
			EUserInterfaceActionType::None);
	}
	MenuBuilder.EndSection();
}

void FDMToolBoxEditorMenu::GenerateGameplayTagAccessors()
{
	FText Message;
	const bool bSucceeded = FDMGameplayTagAccessorGenerator::Generate(Message);
	FMessageDialog::Open(bSucceeded ? EAppMsgType::Ok : EAppMsgType::Ok, Message);
}

#undef LOCTEXT_NAMESPACE
