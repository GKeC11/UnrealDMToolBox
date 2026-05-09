// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"

class UEnhancedDataTable;

/** Excel 单元格文本表格，已按导入配置裁剪为 Header 和有效数据行。 */
struct FEnhancedDataTableExcelData
{
	/** 前两列固定为序号和 RowName，后续为连续读取到的属性字段名。 */
	TArray<FString> Headers;

	/** 每个数据行的原始 Excel 行号。 */
	TArray<int32> SourceRowNumbers;

	/** 每个数据行的单元格文本，列顺序与 Headers 一致。 */
	TArray<TArray<FString>> Rows;
};

/** EnhancedDataTable 的 Editor-only Excel 读取和导入工具。 */
class FEnhancedDataTableUtils
{
public:
	/** 读取 .xlsx 并按资产配置提取 Header 和有效数据行。 */
	static bool ReadXlsx(const UEnhancedDataTable* DataTable, FEnhancedDataTableExcelData& OutExcelData, FText& OutError);

	/** 读取 .xlsx、校验字段和行数据，并整体写入 DataTable。 */
	static bool ImportXlsxToDataTable(UEnhancedDataTable* DataTable, FText& OutError, int32& OutImportedRowCount);
};

#endif // WITH_EDITOR
