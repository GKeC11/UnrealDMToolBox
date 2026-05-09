// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnhancedDataTableUtils.h"

#if WITH_EDITOR

#include "DataTableEditorUtils.h"
#include "EnhancedDataTable.h"
#include "ScopedTransaction.h"
#include "UObject/StructOnScope.h"
#include "UObject/UnrealType.h"

THIRD_PARTY_INCLUDES_START
#include <OpenXLSX.hpp>
#include <exception>
#include <string>
#include <vector>
THIRD_PARTY_INCLUDES_END

#define LOCTEXT_NAMESPACE "EnhancedDataTableUtils"

namespace
{
	FString TrimCellText(const FString& InText)
	{
		return InText.TrimStartAndEnd();
	}

	FString ConvertCellValueToString(const OpenXLSX::XLCellValue& CellValue)
	{
		switch (CellValue.type())
		{
		case OpenXLSX::XLValueType::Empty:
			return FString();
		case OpenXLSX::XLValueType::Boolean:
			return CellValue.get<bool>() ? TEXT("True") : TEXT("False");
		case OpenXLSX::XLValueType::Integer:
			return LexToString(CellValue.get<int64_t>());
		case OpenXLSX::XLValueType::Float:
			return LexToString(CellValue.get<double>());
		case OpenXLSX::XLValueType::String:
			return FString(UTF8_TO_TCHAR(CellValue.get<std::string>().c_str()));
		case OpenXLSX::XLValueType::Error:
			return TEXT("#ERROR");
		default:
			return FString();
		}
	}

	bool IsEmptyDataRow(const TArray<FString>& Cells)
	{
		for (const FString& Cell : Cells)
		{
			if (!TrimCellText(Cell).IsEmpty())
			{
				return false;
			}
		}

		return true;
	}

	void AddError(TArray<FText>& Errors, const FText& Error)
	{
		Errors.Add(Error);
	}

	FText JoinErrors(const TArray<FText>& Errors)
	{
		FString ErrorText;
		for (const FText& Error : Errors)
		{
			if (!ErrorText.IsEmpty())
			{
				ErrorText += LINE_TERMINATOR;
			}

			ErrorText += Error.ToString();
		}

		return FText::FromString(ErrorText);
	}

	bool ValidateImportConfig(const UEnhancedDataTable* DataTable, FText& OutError)
	{
		if (!DataTable)
		{
			OutError = LOCTEXT("MissingDataTable", "Enhanced DataTable is invalid.");
			return false;
		}

		if (DataTable->ExcelFirstDataRow <= 1)
		{
			OutError = LOCTEXT("InvalidFirstDataRow", "ExcelFirstDataRow must be greater than 1 because the header row is ExcelFirstDataRow - 1.");
			return false;
		}

		if (DataTable->ExcelFirstColumn <= 0)
		{
			OutError = LOCTEXT("InvalidFirstColumn", "ExcelFirstColumn must be greater than 0.");
			return false;
		}

		if (DataTable->ExcelFirstColumn + 1 > OpenXLSX::MAX_COLS)
		{
			OutError = FText::Format(LOCTEXT("FirstColumnOutOfRange", "ExcelFirstColumn and the following RowName column must fit within the .xlsx column limit: {0}."), FText::AsNumber(OpenXLSX::MAX_COLS));
			return false;
		}

		if (DataTable->ExcelSourceFile.FilePath.IsEmpty())
		{
			OutError = LOCTEXT("MissingExcelPath", "Choose an Excel file before importing.");
			return false;
		}

		if (!FPaths::FileExists(DataTable->ExcelSourceFile.FilePath))
		{
			OutError = FText::Format(LOCTEXT("ExcelFileMissing", "Excel file does not exist:\n{0}"), FText::FromString(DataTable->ExcelSourceFile.FilePath));
			return false;
		}

		const FString Extension = FPaths::GetExtension(DataTable->ExcelSourceFile.FilePath).ToLower();
		if (Extension == TEXT("xls"))
		{
			OutError = LOCTEXT("XlsUnsupported", ".xls is not supported. Save the file as .xlsx and import again.");
			return false;
		}

		if (Extension != TEXT("xlsx"))
		{
			OutError = LOCTEXT("InvalidExcelExtension", "The selected file must be .xlsx.");
			return false;
		}

		return true;
	}

	bool BuildPropertyMap(const UScriptStruct* RowStruct, TMap<FName, FProperty*>& OutProperties)
	{
		if (!RowStruct)
		{
			return false;
		}

		for (TFieldIterator<FProperty> PropertyIt(RowStruct); PropertyIt; ++PropertyIt)
		{
			FProperty* Property = *PropertyIt;
			if (Property)
			{
				OutProperties.Add(Property->GetFName(), Property);
			}
		}

		return true;
	}
}

bool FEnhancedDataTableUtils::ReadXlsx(const UEnhancedDataTable* DataTable, FEnhancedDataTableExcelData& OutExcelData, FText& OutError)
{
	OutExcelData = FEnhancedDataTableExcelData();

	if (!ValidateImportConfig(DataTable, OutError))
	{
		return false;
	}

	try
	{
		OpenXLSX::XLDocument Document;
		Document.open(TCHAR_TO_UTF8(*DataTable->ExcelSourceFile.FilePath));

		OpenXLSX::XLWorksheet Worksheet;
		const FString SheetName = DataTable->ExcelSheetName.ToString();
		if (SheetName.IsEmpty())
		{
			const std::vector<std::string> WorksheetNames = Document.workbook().worksheetNames();
			if (WorksheetNames.empty())
			{
				Document.close();
				OutError = LOCTEXT("MissingWorksheet", "The workbook does not contain a worksheet.");
				return false;
			}

			Worksheet = Document.workbook().worksheet(WorksheetNames.front());
		}
		else
		{
			const std::string SheetNameUtf8 = TCHAR_TO_UTF8(*SheetName);
			if (!Document.workbook().worksheetExists(SheetNameUtf8))
			{
				Document.close();
				OutError = FText::Format(LOCTEXT("WorksheetMissingByName", "Worksheet does not exist: {0}"), FText::FromString(SheetName));
				return false;
			}

			Worksheet = Document.workbook().worksheet(SheetNameUtf8);
		}

		const uint32 HeaderRow = static_cast<uint32>(DataTable->ExcelFirstDataRow - 1);
		const uint32 FirstDataRow = static_cast<uint32>(DataTable->ExcelFirstDataRow);
		const uint16 FirstColumn = static_cast<uint16>(DataTable->ExcelFirstColumn);
		const uint16 PropertyFirstColumn = static_cast<uint16>(FirstColumn + 2);
		const uint16 LastColumn = Worksheet.columnCount();
		const uint32 LastRow = Worksheet.rowCount();

		if (HeaderRow > LastRow || FirstColumn + 1 > LastColumn)
		{
			Document.close();
			OutError = LOCTEXT("ConfiguredRangeOutsideSheet", "The configured first data row, serial column, or RowName column is outside the used worksheet range.");
			return false;
		}

		OutExcelData.Headers.Add(TEXT("__Index"));
		OutExcelData.Headers.Add(TEXT("RowName"));

		for (uint16 ColumnIndex = PropertyFirstColumn; ColumnIndex <= LastColumn; ++ColumnIndex)
		{
			const FString HeaderText = TrimCellText(ConvertCellValueToString(static_cast<OpenXLSX::XLCellValue>(Worksheet.cell(HeaderRow, ColumnIndex).value())));
			if (HeaderText.IsEmpty())
			{
				break;
			}

			OutExcelData.Headers.Add(HeaderText);
		}

		if (OutExcelData.Headers.IsEmpty())
		{
			Document.close();
			OutError = LOCTEXT("MissingHeaders", "No property headers were found after the serial and RowName columns.");
			return false;
		}

		if (OutExcelData.Headers.Num() <= 2)
		{
			Document.close();
			OutError = LOCTEXT("MissingPropertyHeaders", "No property headers were found after the serial and RowName columns.");
			return false;
		}

		for (uint32 RowIndex = FirstDataRow; RowIndex <= LastRow; ++RowIndex)
		{
			TArray<FString> Cells;
			Cells.Reserve(OutExcelData.Headers.Num());

			for (int32 HeaderIndex = 0; HeaderIndex < OutExcelData.Headers.Num(); ++HeaderIndex)
			{
				const uint16 ColumnIndex = static_cast<uint16>(FirstColumn + HeaderIndex);
				Cells.Add(ConvertCellValueToString(static_cast<OpenXLSX::XLCellValue>(Worksheet.cell(RowIndex, ColumnIndex).value())));
			}

			if (IsEmptyDataRow(Cells))
			{
				break;
			}

			OutExcelData.SourceRowNumbers.Add(static_cast<int32>(RowIndex));
			OutExcelData.Rows.Add(MoveTemp(Cells));
		}

		Document.close();
		return true;
	}
	catch (const std::exception& Exception)
	{
		OutError = FText::Format(LOCTEXT("OpenXlsxException", "Failed to read .xlsx file:\n{0}"), FText::FromString(UTF8_TO_TCHAR(Exception.what())));
		return false;
	}
	catch (...)
	{
		OutError = LOCTEXT("UnknownOpenXlsxException", "Failed to read .xlsx file because OpenXLSX reported an unknown error.");
		return false;
	}
}

bool FEnhancedDataTableUtils::ImportXlsxToDataTable(UEnhancedDataTable* DataTable, FText& OutError, int32& OutImportedRowCount)
{
	OutImportedRowCount = 0;

	if (!DataTable)
	{
		OutError = LOCTEXT("ImportMissingDataTable", "Enhanced DataTable is invalid.");
		return false;
	}

	const UScriptStruct* RowStruct = DataTable->GetRowStruct();
	if (!RowStruct)
	{
		OutError = LOCTEXT("ImportMissingRowStruct", "This Enhanced DataTable has no row structure.");
		return false;
	}

	FEnhancedDataTableExcelData ExcelData;
	if (!ReadXlsx(DataTable, ExcelData, OutError))
	{
		return false;
	}

	TArray<FText> Errors;
	TSet<FString> SeenHeaders;
	TMap<FName, FProperty*> PropertyMap;
	BuildPropertyMap(RowStruct, PropertyMap);

	TArray<FProperty*> ImportProperties;
	ImportProperties.SetNum(ExcelData.Headers.Num());

	for (int32 HeaderIndex = 2; HeaderIndex < ExcelData.Headers.Num(); ++HeaderIndex)
	{
		const FString& Header = ExcelData.Headers[HeaderIndex];
		if (Header == TEXT("RowName") || Header == TEXT("__Index"))
		{
			AddError(Errors, FText::Format(LOCTEXT("ReservedHeader", "Header uses a reserved column name: {0}."), FText::FromString(Header)));
			continue;
		}

		if (SeenHeaders.Contains(Header))
		{
			AddError(Errors, FText::Format(LOCTEXT("DuplicateHeader", "Duplicate header: {0}."), FText::FromString(Header)));
			continue;
		}

		SeenHeaders.Add(Header);

		FProperty* const* PropertyPtr = PropertyMap.Find(FName(*Header));
		if (!PropertyPtr || !*PropertyPtr)
		{
			AddError(Errors, FText::Format(LOCTEXT("UnknownHeader", "Header does not match any row property: {0}."), FText::FromString(Header)));
			continue;
		}

		ImportProperties[HeaderIndex] = *PropertyPtr;
	}

	TSet<FName> SeenRowNames;
	TArray<TPair<FName, TSharedPtr<FStructOnScope>>> PendingRows;

	for (int32 RowIndex = 0; RowIndex < ExcelData.Rows.Num(); ++RowIndex)
	{
		const TArray<FString>& Cells = ExcelData.Rows[RowIndex];
		const int32 SourceRowNumber = ExcelData.SourceRowNumbers.IsValidIndex(RowIndex) ? ExcelData.SourceRowNumbers[RowIndex] : RowIndex + DataTable->ExcelFirstDataRow;
		const FString RowNameText = Cells.IsValidIndex(1) ? TrimCellText(Cells[1]) : FString();

		if (RowNameText.IsEmpty())
		{
			AddError(Errors, FText::Format(LOCTEXT("EmptyRowName", "RowName is empty at Excel row {0}."), FText::AsNumber(SourceRowNumber)));
			continue;
		}

		const FName RowName(*RowNameText);
		if (SeenRowNames.Contains(RowName))
		{
			AddError(Errors, FText::Format(LOCTEXT("DuplicateRowName", "Duplicate RowName '{0}' at Excel row {1}."), FText::FromString(RowNameText), FText::AsNumber(SourceRowNumber)));
			continue;
		}

		SeenRowNames.Add(RowName);

		TSharedPtr<FStructOnScope> RowData = MakeShared<FStructOnScope>(RowStruct);

		for (int32 HeaderIndex = 2; HeaderIndex < ExcelData.Headers.Num(); ++HeaderIndex)
		{
			FProperty* Property = ImportProperties.IsValidIndex(HeaderIndex) ? ImportProperties[HeaderIndex] : nullptr;
			if (!Property)
			{
				continue;
			}

			const FString CellText = Cells.IsValidIndex(HeaderIndex) ? Cells[HeaderIndex] : FString();
			const TCHAR* ImportResult = Property->ImportText_Direct(
				*CellText,
				Property->ContainerPtrToValuePtr<void>(RowData->GetStructMemory()),
				DataTable,
				PPF_None);

			if (!ImportResult)
			{
				AddError(Errors, FText::Format(
					LOCTEXT("CellImportFailed", "Failed to import property '{0}' at Excel row {1}: '{2}'."),
					FText::FromString(Property->GetName()),
					FText::AsNumber(SourceRowNumber),
					FText::FromString(CellText)));
			}
		}

		PendingRows.Emplace(RowName, MoveTemp(RowData));
	}

	if (!Errors.IsEmpty())
	{
		OutError = JoinErrors(Errors);
		return false;
	}

	const FScopedTransaction Transaction(LOCTEXT("ImportEnhancedDataTableXlsx", "Import Enhanced DataTable Excel"));
	DataTable->Modify();
	FDataTableEditorUtils::BroadcastPreChange(DataTable, FDataTableEditorUtils::EDataTableChangeInfo::RowList);
	DataTable->EmptyTable();

	for (TPair<FName, TSharedPtr<FStructOnScope>>& PendingRow : PendingRows)
	{
		DataTable->AddRow(PendingRow.Key, PendingRow.Value->GetStructMemory(), RowStruct);
		DataTable->HandleDataTableChanged(PendingRow.Key);
	}

	DataTable->MarkPackageDirty();
	FDataTableEditorUtils::BroadcastPostChange(DataTable, FDataTableEditorUtils::EDataTableChangeInfo::RowList);

	OutImportedRowCount = PendingRows.Num();
	return true;
}

#undef LOCTEXT_NAMESPACE

#endif // WITH_EDITOR
