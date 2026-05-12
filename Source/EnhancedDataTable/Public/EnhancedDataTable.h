// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataTable.h"
#include "UObject/SoftObjectPath.h"

#include "EnhancedDataTable.generated.h"

/**
 * DataTable asset variant that keeps editor import metadata for an Excel source file.
 *
 * Runtime code can keep using it as a normal UDataTable. Excel parsing and import are
 * owned by editor-only tooling.
 */
UCLASS(BlueprintType, AutoExpandCategories = "DataTable,ImportOptions,ExcelImport", Meta = (LoadBehavior = "LazyOnDemand"))
class ENHANCEDDATATABLE_API UEnhancedDataTable : public UDataTable
{
	GENERATED_BODY()

public:
	/** Excel file selected by the enhanced DataTable editor. */
	UPROPERTY(EditAnywhere, Category = "Excel Import", meta = (FilePathFilter = "Excel files (*.xlsx;*.xls)|*.xlsx;*.xls|All files (*.*)|*.*"))
	FFilePath ExcelSourceFile;

	/** Optional sheet name. Empty means the importer should use its default sheet selection rule. */
	UPROPERTY(EditAnywhere, Category = "Excel Import")
	FName ExcelSheetName;

	/** 第一条有效数据所在行，字段名行固定为上一行。 */
	UPROPERTY(EditAnywhere, Category = "Excel Import", meta = (ClampMin = "2"))
	int32 ExcelFirstDataRow = 2;

	/** 序号列所在列，后一列固定作为 RowName，属性字段从再后一列开始。 */
	UPROPERTY(EditAnywhere, Category = "Excel Import", meta = (ClampMin = "1"))
	int32 ExcelFirstColumn = 1;
};
