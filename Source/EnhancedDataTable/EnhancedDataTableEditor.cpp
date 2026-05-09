// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnhancedDataTableEditor.h"

#if WITH_EDITOR

#include "DesktopPlatformModule.h"
#include "EnhancedDataTable.h"
#include "EnhancedDataTableUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformApplicationMisc.h"
#include "IDesktopPlatform.h"
#include "IStructureDetailsView.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "ScopedTransaction.h"
#include "Styling/AppStyle.h"
#include "UObject/StructOnScope.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "EnhancedDataTableEditor"

const FName FEnhancedDataTableEditor::DataTableTabId(TEXT("EnhancedDataTableEditor_DataTable"));
const FName FEnhancedDataTableEditor::DetailsTabId(TEXT("EnhancedDataTableEditor_Details"));
const FName FEnhancedDataTableEditor::RowNumberColumnId(TEXT("RowNumber"));
const FName FEnhancedDataTableEditor::RowNameColumnId(TEXT("RowName"));

namespace
{
	class FEnhancedDataTableRowStruct final : public FStructOnScope
	{
	public:
		FEnhancedDataTableRowStruct(UEnhancedDataTable* InDataTable, FName InRowName)
			: DataTable(InDataTable)
			, RowName(InRowName)
		{
		}

		virtual uint8* GetStructMemory() override
		{
			return DataTable.IsValid() && !RowName.IsNone() ? DataTable->FindRowUnchecked(RowName) : nullptr;
		}

		virtual const uint8* GetStructMemory() const override
		{
			return DataTable.IsValid() && !RowName.IsNone() ? DataTable->FindRowUnchecked(RowName) : nullptr;
		}

		virtual const UScriptStruct* GetStruct() const override
		{
			return DataTable.IsValid() ? DataTable->GetRowStruct() : nullptr;
		}

		virtual UPackage* GetPackage() const override
		{
			return DataTable.IsValid() ? DataTable->GetOutermost() : nullptr;
		}

		virtual void SetPackage(UPackage* InPackage) override
		{
		}

		virtual bool IsValid() const override
		{
			return DataTable.IsValid()
				&& DataTable->GetRowStruct()
				&& !RowName.IsNone()
				&& DataTable->FindRowUnchecked(RowName);
		}

		virtual void Destroy() override
		{
			DataTable = nullptr;
			RowName = NAME_None;
		}

	private:
		TWeakObjectPtr<UEnhancedDataTable> DataTable;
		FName RowName = NAME_None;
	};

	class SEnhancedDataTableRow final : public SMultiColumnTableRow<FDataTableEditorRowListViewDataPtr>
	{
	public:
		SLATE_BEGIN_ARGS(SEnhancedDataTableRow) {}
			SLATE_ARGUMENT(FDataTableEditorRowListViewDataPtr, RowData)
			SLATE_ARGUMENT(TWeakPtr<FEnhancedDataTableEditor>, Editor)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable)
		{
			RowData = InArgs._RowData;
			Editor = InArgs._Editor;
			SMultiColumnTableRow<FDataTableEditorRowListViewDataPtr>::Construct(
				FSuperRowType::FArguments().Padding(FMargin(2.0f, 1.0f)),
				OwnerTable);
		}

		virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
		{
			FText CellText = FText::GetEmpty();
			if (RowData.IsValid())
			{
				if (ColumnName == FEnhancedDataTableEditor::RowNumberColumnId)
				{
					CellText = FText::AsNumber(RowData->RowNum);
				}
				else if (ColumnName == FEnhancedDataTableEditor::RowNameColumnId)
				{
					CellText = RowData->DisplayName;
				}
				else if (const TSharedPtr<FEnhancedDataTableEditor> EditorPinned = Editor.Pin())
				{
					const TArray<FDataTableEditorColumnHeaderDataPtr>& Columns = EditorPinned->GetAvailableColumns();
					const int32 ColumnIndex = Columns.IndexOfByPredicate([ColumnName](const FDataTableEditorColumnHeaderDataPtr& Column)
					{
						return Column.IsValid() && Column->ColumnId == ColumnName;
					});

					if (ColumnIndex != INDEX_NONE)
					{
						CellText = EditorPinned->GetCellText(RowData, ColumnIndex);
					}
				}
			}

			return SNew(STextBlock)
				.Text(CellText)
				.TextStyle(FAppStyle::Get(), "DataTableEditor.CellText")
				.ToolTipText(CellText);
		}

	private:
		FDataTableEditorRowListViewDataPtr RowData;
		TWeakPtr<FEnhancedDataTableEditor> Editor;
	};
}

FEnhancedDataTableEditor::FEnhancedDataTableEditor() = default;

FEnhancedDataTableEditor::~FEnhancedDataTableEditor()
{
	FDataTableEditorUtils::FDataTableEditorManager::Get().RemoveListener(this);
}

void FEnhancedDataTableEditor::InitEnhancedDataTableEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UEnhancedDataTable* Table)
{
	EnhancedDataTable = Table;
	FDataTableEditorUtils::FDataTableEditorManager::Get().AddListener(this);
	RefreshCachedDataTable();

	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("EnhancedDataTableEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.75f)
				->AddTab(DataTableTabId, ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.25f)
				->AddTab(DetailsTabId, ETabState::OpenedTab)
			)
		);

	constexpr bool bCreateDefaultStandaloneMenu = true;
	constexpr bool bCreateDefaultToolbar = true;
	InitAssetEditor(Mode, InitToolkitHost, TEXT("EnhancedDataTableEditorApp"), StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, Table);

	TSharedPtr<FExtender> ToolbarExtender = MakeShared<FExtender>();
	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP(this, &FEnhancedDataTableEditor::FillToolbar));
	AddToolbarExtender(ToolbarExtender);
	RegenerateMenusAndToolbars();
}

FText FEnhancedDataTableEditor::GetCellText(FDataTableEditorRowListViewDataPtr RowData, int32 ColumnIndex) const
{
	if (RowData.IsValid() && RowData->CellData.IsValidIndex(ColumnIndex))
	{
		return RowData->CellData[ColumnIndex];
	}

	return FText::GetEmpty();
}

FText FEnhancedDataTableEditor::GetFilterText() const
{
	return FilterText;
}

FName FEnhancedDataTableEditor::GetToolkitFName() const
{
	return FName("EnhancedDataTableEditor");
}

FText FEnhancedDataTableEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Enhanced Data Table");
}

FString FEnhancedDataTableEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Enhanced Data Table ").ToString();
}

FLinearColor FEnhancedDataTableEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(FColor(62, 140, 35));
}

void FEnhancedDataTableEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu", "Enhanced Data Table"));
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(DataTableTabId, FOnSpawnTab::CreateSP(this, &FEnhancedDataTableEditor::SpawnTab_DataTable))
		.SetDisplayName(LOCTEXT("DataTableTab", "Data Table"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());

	InTabManager->RegisterTabSpawner(DetailsTabId, FOnSpawnTab::CreateSP(this, &FEnhancedDataTableEditor::SpawnTab_RowEditor))
		.SetDisplayName(LOCTEXT("RowEditorTab", "Row Editor"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FEnhancedDataTableEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner(DataTableTabId);
	InTabManager->UnregisterTabSpawner(DetailsTabId);
}

void FEnhancedDataTableEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(EnhancedDataTable);
}

FString FEnhancedDataTableEditor::GetReferencerName() const
{
	return TEXT("FEnhancedDataTableEditor");
}

void FEnhancedDataTableEditor::NotifyPreChange(FProperty* PropertyAboutToChange)
{
	if (!EnhancedDataTable)
	{
		return;
	}

	EnhancedDataTable->Modify();
	FDataTableEditorUtils::BroadcastPreChange(EnhancedDataTable, FDataTableEditorUtils::EDataTableChangeInfo::RowData);
}

void FEnhancedDataTableEditor::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged)
{
	if (!EnhancedDataTable)
	{
		return;
	}

	EnhancedDataTable->HandleDataTableChanged(HighlightedRowName);
	EnhancedDataTable->MarkPackageDirty();
	FDataTableEditorUtils::BroadcastPostChange(EnhancedDataTable, FDataTableEditorUtils::EDataTableChangeInfo::RowData);
}

void FEnhancedDataTableEditor::PreChange(const UDataTable* Changed, FDataTableEditorUtils::EDataTableChangeInfo Info)
{
}

void FEnhancedDataTableEditor::PostChange(const UDataTable* Changed, FDataTableEditorUtils::EDataTableChangeInfo Info)
{
	if (Changed == EnhancedDataTable)
	{
		RefreshCachedDataTable();
		RefreshRowEditor();
	}
}

void FEnhancedDataTableEditor::SelectionChange(const UDataTable* DataTable, FName RowName)
{
	if (DataTable == EnhancedDataTable)
	{
		SelectRowByName(RowName);
	}
}

TSharedRef<SDockTab> FEnhancedDataTableEditor::SpawnTab_DataTable(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("DataTableTab", "Data Table"))
		[
			CreateDataTableWidget()
		];
}

TSharedRef<SDockTab> FEnhancedDataTableEditor::SpawnTab_RowEditor(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("RowEditorTitle", "Row Editor"))
		[
			CreateRowEditorWidget()
		];
}

TSharedRef<SWidget> FEnhancedDataTableEditor::CreateDataTableWidget()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SAssignNew(SearchBox, SSearchBox)
			.HintText(LOCTEXT("SearchHint", "Search"))
			.OnTextChanged(this, &FEnhancedDataTableEditor::OnFilterTextChanged)
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(RowListView, SListView<FDataTableEditorRowListViewDataPtr>)
			.ListItemsSource(&VisibleRows)
			.HeaderRow(MakeHeaderRow())
			.OnGenerateRow(this, &FEnhancedDataTableEditor::MakeRowWidget)
			.OnSelectionChanged(this, &FEnhancedDataTableEditor::OnRowSelectionChanged)
			.SelectionMode(ESelectionMode::Single)
		];
}

TSharedRef<SWidget> FEnhancedDataTableEditor::CreateRowEditorWidget()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bShowObjectLabel = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.NotifyHook = this;

	FStructureDetailsViewArgs StructureViewArgs;
	StructureViewArgs.bShowObjects = false;
	StructureViewArgs.bShowAssets = true;
	StructureViewArgs.bShowClasses = true;
	StructureViewArgs.bShowInterfaces = false;

	RowDetailsView = PropertyEditorModule.CreateStructureDetailView(DetailsViewArgs, StructureViewArgs, nullptr, LOCTEXT("RowValue", "Row Value"));
	RefreshRowEditor();

	return RowDetailsView->GetWidget().ToSharedRef();
}

TSharedRef<ITableRow> FEnhancedDataTableEditor::MakeRowWidget(FDataTableEditorRowListViewDataPtr RowData, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SEnhancedDataTableRow, OwnerTable)
		.RowData(RowData)
		.Editor(SharedThis(this));
}

TSharedRef<SHeaderRow> FEnhancedDataTableEditor::MakeHeaderRow()
{
	TSharedRef<SHeaderRow> HeaderRow = SNew(SHeaderRow);

	HeaderRow->AddColumn(
		SHeaderRow::Column(RowNumberColumnId)
		.FixedWidth(48.0f)
		.DefaultLabel(LOCTEXT("RowNumberColumn", "#")));

	HeaderRow->AddColumn(
		SHeaderRow::Column(RowNameColumnId)
		.ManualWidth(160.0f)
		.DefaultLabel(LOCTEXT("RowNameColumn", "Row Name")));

	for (const FDataTableEditorColumnHeaderDataPtr& Column : AvailableColumns)
	{
		if (Column.IsValid())
		{
			HeaderRow->AddColumn(
				SHeaderRow::Column(Column->ColumnId)
				.ManualWidth(FMath::Clamp(Column->DesiredColumnWidth, 80.0f, 480.0f))
				.DefaultLabel(Column->DisplayName));
		}
	}

	return HeaderRow;
}

void FEnhancedDataTableEditor::RefreshCachedDataTable()
{
	FDataTableEditorUtils::CacheDataTableForEditing(EnhancedDataTable, AvailableColumns, AvailableRows);
	UpdateVisibleRows();

	if (RowListView.IsValid())
	{
		RowListView->RequestListRefresh();
		SelectRowByName(HighlightedRowName);
	}
}

void FEnhancedDataTableEditor::RefreshRowEditor()
{
	if (!RowDetailsView.IsValid())
	{
		return;
	}

	if (CurrentRow.IsValid())
	{
		CurrentRow->Destroy();
		CurrentRow.Reset();
	}

	if (!EnhancedDataTable || HighlightedRowName.IsNone() || !EnhancedDataTable->FindRowUnchecked(HighlightedRowName))
	{
		RowDetailsView->SetCustomName(FText::GetEmpty());
		RowDetailsView->SetStructureData(nullptr);
		return;
	}

	CurrentRow = MakeShared<FEnhancedDataTableRowStruct>(EnhancedDataTable, HighlightedRowName);
	RowDetailsView->SetCustomName(FText::FromName(HighlightedRowName));
	RowDetailsView->SetStructureData(CurrentRow);
}

void FEnhancedDataTableEditor::UpdateVisibleRows()
{
	VisibleRows.Reset();
	const FString FilterString = FilterText.ToString();

	for (const FDataTableEditorRowListViewDataPtr& Row : AvailableRows)
	{
		if (!Row.IsValid())
		{
			continue;
		}

		bool bPassesFilter = FilterString.IsEmpty() || Row->DisplayName.ToString().Contains(FilterString);
		if (!bPassesFilter)
		{
			for (const FText& Cell : Row->CellData)
			{
				if (Cell.ToString().Contains(FilterString))
				{
					bPassesFilter = true;
					break;
				}
			}
		}

		if (bPassesFilter)
		{
			VisibleRows.Add(Row);
		}
	}
}

void FEnhancedDataTableEditor::OnFilterTextChanged(const FText& InFilterText)
{
	FilterText = InFilterText;
	UpdateVisibleRows();

	if (RowListView.IsValid())
	{
		RowListView->RequestListRefresh();
	}
}

void FEnhancedDataTableEditor::OnRowSelectionChanged(FDataTableEditorRowListViewDataPtr NewSelection, ESelectInfo::Type SelectInfo)
{
	HighlightedRowName = NewSelection.IsValid() ? NewSelection->RowId : NAME_None;
	RefreshRowEditor();
}

void FEnhancedDataTableEditor::FillToolbar(FToolBarBuilder& ToolbarBuilder)
{
	ToolbarBuilder.BeginSection("EnhancedDataTableCommands");
	{
		ToolbarBuilder.AddToolBarButton(
			FUIAction(FExecuteAction::CreateSP(this, &FEnhancedDataTableEditor::AddRow)),
			NAME_None,
			LOCTEXT("AddIconText", "Add"),
			LOCTEXT("AddRowToolTip", "Add a new row to the Data Table"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Plus"));

		ToolbarBuilder.AddToolBarButton(
			FUIAction(
				FExecuteAction::CreateSP(this, &FEnhancedDataTableEditor::CopySelectedRow),
				FCanExecuteAction::CreateSP(this, &FEnhancedDataTableEditor::CanEditSelectedRow)),
			NAME_None,
			LOCTEXT("CopyIconText", "Copy"),
			LOCTEXT("CopyToolTip", "Copy the currently selected row"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "GenericCommands.Copy"));

		ToolbarBuilder.AddToolBarButton(
			FUIAction(
				FExecuteAction::CreateSP(this, &FEnhancedDataTableEditor::PasteOnSelectedRow),
				FCanExecuteAction::CreateSP(this, &FEnhancedDataTableEditor::CanEditSelectedRow)),
			NAME_None,
			LOCTEXT("PasteIconText", "Paste"),
			LOCTEXT("PasteToolTip", "Paste on the currently selected row"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "GenericCommands.Paste"));

		ToolbarBuilder.AddToolBarButton(
			FUIAction(
				FExecuteAction::CreateSP(this, &FEnhancedDataTableEditor::DuplicateSelectedRow),
				FCanExecuteAction::CreateSP(this, &FEnhancedDataTableEditor::CanEditSelectedRow)),
			NAME_None,
			LOCTEXT("DuplicateIconText", "Duplicate"),
			LOCTEXT("DuplicateToolTip", "Duplicate the currently selected row"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Duplicate"));

		ToolbarBuilder.AddToolBarButton(
			FUIAction(
				FExecuteAction::CreateSP(this, &FEnhancedDataTableEditor::RemoveSelectedRow),
				FCanExecuteAction::CreateSP(this, &FEnhancedDataTableEditor::CanEditSelectedRow)),
			NAME_None,
			LOCTEXT("RemoveRowIconText", "Remove"),
			LOCTEXT("RemoveRowToolTip", "Remove the currently selected row from the Data Table"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Delete"));
	}
	ToolbarBuilder.EndSection();

	ToolbarBuilder.BeginSection("EnhancedDataTableExcel");
	{
		ToolbarBuilder.AddSeparator();
		ToolbarBuilder.AddWidget(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ExcelLabel", "Excel"))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(360.0f)
				[
					SNew(SEditableTextBox)
					.HintText(LOCTEXT("ExcelPathHint", "Select an .xlsx or .xls file"))
					.Text_Lambda([this]()
					{
						return EnhancedDataTable ? FText::FromString(EnhancedDataTable->ExcelSourceFile.FilePath) : FText::GetEmpty();
					})
					.OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type)
					{
						SetExcelSourceFile(NewText.ToString());
					})
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ToolTipText(LOCTEXT("BrowseExcelTooltip", "Choose the Excel source file."))
				.OnClicked_Lambda([this]()
				{
					BrowseExcelFile();
					return FReply::Handled();
				})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("BrowseExcelButton", "..."))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ExcelSheetLabel", "Sheet"))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(120.0f)
				[
					SNew(SEditableTextBox)
					.HintText(LOCTEXT("ExcelSheetHint", "Default"))
					.Text_Lambda([this]()
					{
						return EnhancedDataTable && !EnhancedDataTable->ExcelSheetName.IsNone() ? FText::FromName(EnhancedDataTable->ExcelSheetName) : FText::GetEmpty();
					})
					.OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type)
					{
						SetExcelSheetName(NewText.ToString());
					})
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ExcelFirstDataRowLabel", "Row"))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(56.0f)
				[
					SNew(SNumericEntryBox<int32>)
					.MinValue(2)
					.MinSliderValue(2)
					.AllowSpin(true)
					.ToolTipText(LOCTEXT("ExcelFirstDataRowTooltip", "First Excel data row. The header row is one row above it."))
					.Value_Lambda([this]() -> TOptional<int32>
					{
						return EnhancedDataTable ? EnhancedDataTable->ExcelFirstDataRow : TOptional<int32>();
					})
					.OnValueCommitted_Lambda([this](int32 NewValue, ETextCommit::Type)
					{
						SetExcelFirstDataRow(NewValue);
					})
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ExcelFirstColumnLabel", "Col"))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(56.0f)
				[
					SNew(SNumericEntryBox<int32>)
					.MinValue(1)
					.MinSliderValue(1)
					.AllowSpin(true)
					.ToolTipText(LOCTEXT("ExcelFirstColumnTooltip", "First Excel column. The next column is RowName; property columns start after that."))
					.Value_Lambda([this]() -> TOptional<int32>
					{
						return EnhancedDataTable ? EnhancedDataTable->ExcelFirstColumn : TOptional<int32>();
					})
					.OnValueCommitted_Lambda([this](int32 NewValue, ETextCommit::Type)
					{
						SetExcelFirstColumn(NewValue);
					})
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ImportExcelButton", "Import"))
				.IsEnabled_Lambda([this]() { return HasExcelSourceFile(); })
				.OnClicked_Lambda([this]()
				{
					ImportExcel();
					return FReply::Handled();
				})
			]);
	}
	ToolbarBuilder.EndSection();
}

void FEnhancedDataTableEditor::AddRow()
{
	if (!EnhancedDataTable || !EnhancedDataTable->GetRowStruct())
	{
		return;
	}

	FName NewName = DataTableUtils::MakeValidName(TEXT("NewRow"));
	while (EnhancedDataTable->GetRowMap().Contains(NewName))
	{
		NewName.SetNumber(NewName.GetNumber() + 1);
	}

	FDataTableEditorUtils::AddRow(EnhancedDataTable, NewName);
	FDataTableEditorUtils::SelectRow(EnhancedDataTable, NewName);
}

void FEnhancedDataTableEditor::CopySelectedRow()
{
	if (!EnhancedDataTable || HighlightedRowName == NAME_None || !EnhancedDataTable->GetRowStruct())
	{
		return;
	}

	uint8* const RowPtr = EnhancedDataTable->GetRowMap().FindRef(HighlightedRowName);
	if (!RowPtr)
	{
		return;
	}

	FString ClipboardValue;
	EnhancedDataTable->GetRowStruct()->ExportText(ClipboardValue, RowPtr, RowPtr, EnhancedDataTable, PPF_Copy, nullptr);
	FPlatformApplicationMisc::ClipboardCopy(*ClipboardValue);
}

void FEnhancedDataTableEditor::PasteOnSelectedRow()
{
	if (!EnhancedDataTable || HighlightedRowName == NAME_None || !EnhancedDataTable->GetRowStruct())
	{
		return;
	}

	uint8* const RowPtr = EnhancedDataTable->GetRowMap().FindRef(HighlightedRowName);
	if (!RowPtr)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("PasteDataTableRow", "Paste Data Table Row"));
	EnhancedDataTable->Modify();

	FString ClipboardValue;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardValue);

	FDataTableEditorUtils::BroadcastPreChange(EnhancedDataTable, FDataTableEditorUtils::EDataTableChangeInfo::RowData);
	const TCHAR* Result = EnhancedDataTable->GetRowStruct()->ImportText(*ClipboardValue, RowPtr, EnhancedDataTable, PPF_Copy, GWarn, GetPathNameSafe(EnhancedDataTable->GetRowStruct()));
	EnhancedDataTable->HandleDataTableChanged(HighlightedRowName);
	EnhancedDataTable->MarkPackageDirty();
	FDataTableEditorUtils::BroadcastPostChange(EnhancedDataTable, FDataTableEditorUtils::EDataTableChangeInfo::RowData);

	if (Result == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FailedPaste", "Failed to paste row."));
	}
}

void FEnhancedDataTableEditor::DuplicateSelectedRow()
{
	if (!EnhancedDataTable || HighlightedRowName == NAME_None)
	{
		return;
	}

	FName NewName = HighlightedRowName;
	const TArray<FName> ExistingNames = EnhancedDataTable->GetRowNames();
	while (ExistingNames.Contains(NewName))
	{
		NewName.SetNumber(NewName.GetNumber() + 1);
	}

	FDataTableEditorUtils::DuplicateRow(EnhancedDataTable, HighlightedRowName, NewName);
	FDataTableEditorUtils::SelectRow(EnhancedDataTable, NewName);
}

void FEnhancedDataTableEditor::RemoveSelectedRow()
{
	if (!EnhancedDataTable || HighlightedRowName == NAME_None)
	{
		return;
	}

	const int32 RowToRemoveIndex = VisibleRows.IndexOfByPredicate([this](const FDataTableEditorRowListViewDataPtr& Row)
	{
		return Row.IsValid() && Row->RowId == HighlightedRowName;
	});

	if (FDataTableEditorUtils::RemoveRow(EnhancedDataTable, HighlightedRowName))
	{
		const int32 RowIndexToSelect = FMath::Clamp(RowToRemoveIndex, 0, VisibleRows.Num() - 1);
		if (VisibleRows.IsValidIndex(RowIndexToSelect))
		{
			FDataTableEditorUtils::SelectRow(EnhancedDataTable, VisibleRows[RowIndexToSelect]->RowId);
		}
		else
		{
			HighlightedRowName = NAME_None;
			if (RowListView.IsValid())
			{
				RowListView->RequestListRefresh();
			}
		}
	}
}

bool FEnhancedDataTableEditor::CanEditSelectedRow() const
{
	return EnhancedDataTable && HighlightedRowName != NAME_None;
}

void FEnhancedDataTableEditor::SelectRowByName(FName RowName)
{
	HighlightedRowName = RowName;

	if (!RowListView.IsValid())
	{
		return;
	}

	FDataTableEditorRowListViewDataPtr* RowPtr = VisibleRows.FindByPredicate([RowName](const FDataTableEditorRowListViewDataPtr& Row)
	{
		return Row.IsValid() && Row->RowId == RowName;
	});

	if (RowPtr)
	{
		RowListView->SetSelection(*RowPtr);
		RowListView->RequestScrollIntoView(*RowPtr);
	}
	else
	{
		RowListView->ClearSelection();
	}

	RefreshRowEditor();
}

void FEnhancedDataTableEditor::BrowseExcelFile()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform || !EnhancedDataTable)
	{
		return;
	}

	const FString CurrentPath = EnhancedDataTable->ExcelSourceFile.FilePath;
	const FString DefaultPath = CurrentPath.IsEmpty() ? FPaths::ProjectDir() : FPaths::GetPath(CurrentPath);
	TArray<FString> OutFilenames;

	const bool bOpened = DesktopPlatform->OpenFileDialog(
		FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
		LOCTEXT("ChooseExcelFileTitle", "Choose Excel File").ToString(),
		DefaultPath,
		FPaths::GetCleanFilename(CurrentPath),
		TEXT("Excel files (*.xlsx;*.xls)|*.xlsx;*.xls|All files (*.*)|*.*"),
		EFileDialogFlags::None,
		OutFilenames);

	if (bOpened && OutFilenames.Num() > 0)
	{
		SetExcelSourceFile(OutFilenames[0]);
	}
}

void FEnhancedDataTableEditor::ImportExcel()
{
	if (!EnhancedDataTable)
	{
		return;
	}

	FText Error;
	int32 ImportedRowCount = 0;
	if (!FEnhancedDataTableUtils::ImportXlsxToDataTable(EnhancedDataTable, Error, ImportedRowCount))
	{
		FMessageDialog::Open(EAppMsgType::Ok, Error);
		return;
	}

	RefreshCachedDataTable();
	FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("ImportExcelSucceeded", "Imported {0} rows from Excel."), FText::AsNumber(ImportedRowCount)));
}

bool FEnhancedDataTableEditor::HasExcelSourceFile() const
{
	return EnhancedDataTable && !EnhancedDataTable->ExcelSourceFile.FilePath.IsEmpty();
}

void FEnhancedDataTableEditor::SetExcelSourceFile(const FString& InFilePath)
{
	if (!EnhancedDataTable || EnhancedDataTable->ExcelSourceFile.FilePath == InFilePath)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("SetExcelSourceFileTransaction", "Set Excel Source File"));
	EnhancedDataTable->Modify();
	EnhancedDataTable->ExcelSourceFile.FilePath = InFilePath;
	EnhancedDataTable->MarkPackageDirty();
}

void FEnhancedDataTableEditor::SetExcelSheetName(const FString& InSheetName)
{
	const FName NewSheetName = InSheetName.IsEmpty() ? NAME_None : FName(*InSheetName);
	if (!EnhancedDataTable || EnhancedDataTable->ExcelSheetName == NewSheetName)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("SetExcelSheetNameTransaction", "Set Excel Sheet Name"));
	EnhancedDataTable->Modify();
	EnhancedDataTable->ExcelSheetName = NewSheetName;
	EnhancedDataTable->MarkPackageDirty();
}

void FEnhancedDataTableEditor::SetExcelFirstDataRow(int32 InFirstDataRow)
{
	if (!EnhancedDataTable)
	{
		return;
	}

	const int32 NewFirstDataRow = FMath::Max(2, InFirstDataRow);
	if (EnhancedDataTable->ExcelFirstDataRow == NewFirstDataRow)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("SetExcelFirstDataRowTransaction", "Set Excel First Data Row"));
	EnhancedDataTable->Modify();
	EnhancedDataTable->ExcelFirstDataRow = NewFirstDataRow;
	EnhancedDataTable->MarkPackageDirty();
}

void FEnhancedDataTableEditor::SetExcelFirstColumn(int32 InFirstColumn)
{
	if (!EnhancedDataTable)
	{
		return;
	}

	const int32 NewFirstColumn = FMath::Max(1, InFirstColumn);
	if (EnhancedDataTable->ExcelFirstColumn == NewFirstColumn)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("SetExcelFirstColumnTransaction", "Set Excel First Column"));
	EnhancedDataTable->Modify();
	EnhancedDataTable->ExcelFirstColumn = NewFirstColumn;
	EnhancedDataTable->MarkPackageDirty();
}

#undef LOCTEXT_NAMESPACE

#endif // WITH_EDITOR
