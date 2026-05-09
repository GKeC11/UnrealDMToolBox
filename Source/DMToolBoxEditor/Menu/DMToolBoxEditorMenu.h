#pragma once

#include "CoreMinimal.h"

class FExtender;
class FMenuBarBuilder;
class FMenuBuilder;

class FDMToolBoxEditorMenu
{
public:
	void Register();
	void Unregister();

private:
	void AddMenuBarExtension(FMenuBarBuilder& MenuBarBuilder);
	void FillPulldownMenu(FMenuBuilder& MenuBuilder);
	void GenerateGameplayTagAccessors();

	TSharedPtr<FExtender> MenuBarExtender;
};
