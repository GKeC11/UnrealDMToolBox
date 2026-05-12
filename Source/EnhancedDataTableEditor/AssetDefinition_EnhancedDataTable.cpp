// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetDefinition_EnhancedDataTable.h"

#if WITH_EDITOR

#include "EnhancedDataTable.h"
#include "EnhancedDataTableEditor.h"
#include "Misc/MessageDialog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_EnhancedDataTable)

#define LOCTEXT_NAMESPACE "AssetDefinition_EnhancedDataTable"

FText UAssetDefinition_EnhancedDataTable::GetAssetDisplayName() const
{
	return LOCTEXT("EnhancedDataTable", "Enhanced Data Table");
}

FLinearColor UAssetDefinition_EnhancedDataTable::GetAssetColor() const
{
	return FLinearColor(FColor(62, 140, 35));
}

TSoftClassPtr<UObject> UAssetDefinition_EnhancedDataTable::GetAssetClass() const
{
	return UEnhancedDataTable::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_EnhancedDataTable::GetAssetCategories() const
{
	static const auto Categories = { EAssetCategoryPaths::Misc };
	return Categories;
}

EAssetCommandResult UAssetDefinition_EnhancedDataTable::OpenAssets(const FAssetOpenArgs& OpenArgs) const
{
	TArray<UEnhancedDataTable*> TablesToOpen;
	TArray<UEnhancedDataTable*> InvalidTables;

	for (UEnhancedDataTable* Table : OpenArgs.LoadObjects<UEnhancedDataTable>())
	{
		if (Table->GetRowStruct())
		{
			TablesToOpen.Add(Table);
		}
		else
		{
			InvalidTables.Add(Table);
		}
	}

	if (InvalidTables.Num() > 0)
	{
		const EAppReturnType::Type Result = FMessageDialog::Open(
			EAppMsgType::YesNoCancel,
			LOCTEXT("MissingRowStructMessage", "One or more Enhanced DataTables are missing a row structure and may not be editable. Open them anyway?"),
			LOCTEXT("MissingRowStructTitle", "Continue?"));

		if (Result == EAppReturnType::Cancel)
		{
			return EAssetCommandResult::Handled;
		}

		if (Result == EAppReturnType::Yes)
		{
			TablesToOpen.Append(InvalidTables);
		}
	}

	for (UEnhancedDataTable* Table : TablesToOpen)
	{
		const TSharedRef<FEnhancedDataTableEditor> Editor = MakeShared<FEnhancedDataTableEditor>();
		Editor->InitEnhancedDataTableEditor(OpenArgs.GetToolkitMode(), OpenArgs.ToolkitHost, Table);
	}

	return EAssetCommandResult::Handled;
}

#undef LOCTEXT_NAMESPACE

#endif // WITH_EDITOR
