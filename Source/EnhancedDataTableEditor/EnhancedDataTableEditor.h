// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#if WITH_EDITOR

#include "DataTableEditorUtils.h"
#include "Misc/NotifyHook.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "UObject/GCObject.h"

class FStructOnScope;
class IStructureDetailsView;
class SHeaderRow;
class SSearchBox;
class UEnhancedDataTable;

class FEnhancedDataTableEditor final
	: public FAssetEditorToolkit
	, public FGCObject
	, public FNotifyHook
	, public FDataTableEditorUtils::INotifyOnDataTableChanged
{
public:
	static const FName DataTableTabId;
	static const FName DetailsTabId;
	static const FName RowNumberColumnId;
	static const FName RowNameColumnId;

	FEnhancedDataTableEditor();
	virtual ~FEnhancedDataTableEditor() override;

	void InitEnhancedDataTableEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UEnhancedDataTable* Table);

	const TArray<FDataTableEditorColumnHeaderDataPtr>& GetAvailableColumns() const { return AvailableColumns; }
	FText GetCellText(FDataTableEditorRowListViewDataPtr RowData, int32 ColumnIndex) const;
	FText GetFilterText() const;

	// FAssetEditorToolkit
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

	// FGCObject
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;

	// FNotifyHook
	virtual void NotifyPreChange(FProperty* PropertyAboutToChange) override;
	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;

	// FDataTableEditorUtils::INotifyOnDataTableChanged
	virtual void PreChange(const UDataTable* Changed, FDataTableEditorUtils::EDataTableChangeInfo Info) override;
	virtual void PostChange(const UDataTable* Changed, FDataTableEditorUtils::EDataTableChangeInfo Info) override;
	virtual void SelectionChange(const UDataTable* DataTable, FName RowName) override;

private:
	UEnhancedDataTable* GetEnhancedDataTable() const { return EnhancedDataTable; }

	TSharedRef<SDockTab> SpawnTab_DataTable(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_RowEditor(const FSpawnTabArgs& Args);
	TSharedRef<SWidget> CreateDataTableWidget();
	TSharedRef<SWidget> CreateRowEditorWidget();
	TSharedRef<ITableRow> MakeRowWidget(FDataTableEditorRowListViewDataPtr RowData, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<SHeaderRow> MakeHeaderRow();
	void RefreshCachedDataTable();
	void RefreshRowEditor();
	void UpdateVisibleRows();
	void OnFilterTextChanged(const FText& InFilterText);
	void OnRowSelectionChanged(FDataTableEditorRowListViewDataPtr NewSelection, ESelectInfo::Type SelectInfo);
	void FillToolbar(FToolBarBuilder& ToolbarBuilder);
	void AddRow();
	void CopySelectedRow();
	void PasteOnSelectedRow();
	void DuplicateSelectedRow();
	void RemoveSelectedRow();
	bool CanEditSelectedRow() const;
	void SelectRowByName(FName RowName);
	void BrowseExcelFile();
	void ImportExcel();
	bool HasExcelSourceFile() const;
	void SetExcelSourceFile(const FString& InFilePath);
	void SetExcelSheetName(const FString& InSheetName);
	void SetExcelFirstDataRow(int32 InFirstDataRow);
	void SetExcelFirstColumn(int32 InFirstColumn);

	TObjectPtr<UEnhancedDataTable> EnhancedDataTable = nullptr;
	TArray<FDataTableEditorColumnHeaderDataPtr> AvailableColumns;
	TArray<FDataTableEditorRowListViewDataPtr> AvailableRows;
	TArray<FDataTableEditorRowListViewDataPtr> VisibleRows;
	TSharedPtr<SListView<FDataTableEditorRowListViewDataPtr>> RowListView;
	TSharedPtr<SSearchBox> SearchBox;
	TSharedPtr<IStructureDetailsView> RowDetailsView;
	TSharedPtr<FStructOnScope> CurrentRow;
	FText FilterText;
	FName HighlightedRowName = NAME_None;
};

#endif // WITH_EDITOR
