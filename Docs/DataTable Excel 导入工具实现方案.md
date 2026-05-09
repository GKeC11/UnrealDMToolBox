# EnhancedDataTable Excel 导入工具实现方案

## 1. 目标、原则和首版范围

本方案在 `DMToolBox` 插件中实现增强版 DataTable 资产和编辑器，让项目可以从 `.xlsx` 文件导入数据到 `UEnhancedDataTable`。

### 1.1 目标和原则

- `UEnhancedDataTable` 继承 `UDataTable`，运行时仍按普通 DataTable 使用。
- Excel 读取和导入流程只属于 Editor 工具，不进入游戏运行时包。
- 不修改引擎 `DataTableEditor` 源码，也不直接继承引擎 Private API `FDataTableEditor`。
- 首版只支持 `.xlsx`，`.xls` 导入时提示用户另存为 `.xlsx`。
- 首版以稳定导入为目标，不处理样式、公式计算和合并单元格语义。

普通 `UDataTable` 不受影响，继续使用 UE 原生 `FDataTableEditor`。

### 1.2 首版范围

当前第一版目标：

- 可以创建 `UEnhancedDataTable` 资产。
- 创建资产时复用 UE 原生 DataTable 的 `RowStruct` 选择流程。
- 双击 `UEnhancedDataTable` 时打开 `FEnhancedDataTableEditor`。
- 增强编辑器提供 `Add / Copy / Paste / Duplicate / Remove` 等基础行操作。
- 增强编辑器工具栏提供 Excel 路径选择和 `Import` 按钮。
- Excel 路径保存到 `UEnhancedDataTable::ExcelSourceFile`。
- `Import` 接入 `EnhancedDataTableUtils` 后读取 `.xlsx` 并写入 DataTable。

### 1.3 非目标

当前明确不做：

- 不支持运行时读取 Excel。
- 不支持 `.xls` 老二进制格式。
- 不实现公式计算。
- 不读取或导入单元格样式。
- 不按合并单元格语义展开数据。
- 不复刻 Diff、`CompositeDataTable` 专用逻辑、完整菜单导出等首版不需要的编辑器能力。

## 2. 模块归属、依赖和代码结构

### 2.1 模块归属和职责

当前实现归属 `DMToolBox` 插件的 `EnhancedDataTable` 模块：

```text
Plugins/DMToolBox/Source/EnhancedDataTable/
```

`EnhancedDataTable` 模块职责：

- 定义 `UEnhancedDataTable`。
- 定义 Editor-only 的 Factory、AssetDefinition 和 Editor Toolkit。
- 集成 Editor-only 的 `.xlsx` 读取能力。
- 提供 `EnhancedDataTableUtils`，负责表格解析、校验和 DataTable 写入。

### 2.2 模块依赖

当前必需依赖：

- `Core`：基础类型。
- `CoreUObject`：`UObject`、反射和属性系统。
- `Engine`：`UDataTable`、`FTableRowBase` 等 DataTable 运行时类型。

当前 Editor 构建下额外依赖：

- `ApplicationCore`：编辑器交互基础能力。
- `AssetDefinition`：注册 `UEnhancedDataTable` 的资产打开行为。
- `DesktopPlatform`：选择 Excel 文件。
- `InputCore`：编辑器命令和输入。
- `PropertyEditor`：详情面板。
- `Slate`、`SlateCore`：编辑器 UI。
- `UnrealEd`：DataTable 编辑器工具函数和 Editor-only 能力。

后续新增第三方依赖：

- `OpenXLSX`：只在 `Target.bBuildEditor` 下用于读取 `.xlsx`。
- `OpenXLSX` 所需依赖库按官方构建依赖先完整导入并编译验证，当前重点关注 `pugixml`、`miniz`，以及 Windows 文件名兼容场景下可能涉及的 `nowide`。
- 第三方依赖只服务 Editor 导入流程，验证通过后裁剪为实际读取 `.xlsx` 需要的源码、头文件、许可证和版本记录，不保留示例、测试、文档、Benchmark 等无关文件。

明确不新增到运行时包的依赖：

- 不把 `OpenXLSX` 加到非 Editor 构建路径。
- 不为了导入工具给项目运行时模块增加 Excel、压缩或文件解析依赖。

### 2.3 目标代码结构

已有文件：

```text
Plugins/DMToolBox/Source/EnhancedDataTable/
  AssetDefinition_EnhancedDataTable.h
  AssetDefinition_EnhancedDataTable.cpp
  EnhancedDataTable.h
  EnhancedDataTable.cpp
  EnhancedDataTable.Build.cs
  EnhancedDataTableEditor.h
  EnhancedDataTableEditor.cpp
  EnhancedDataTableFactory.h
  EnhancedDataTableFactory.cpp
  EnhancedDataTableModule.cpp
```

后续新增文件：

```text
Plugins/DMToolBox/Source/EnhancedDataTable/
  EnhancedDataTableUtils.h
  EnhancedDataTableUtils.cpp
```

第三方库建议路径：

```text
Plugins/DMToolBox/ThirdParty/OpenXLSX/
  OpenXLSX/
  external/
    miniz/
    pugixml/
    nowide/       # 仅在最终确认 Windows 路径兼容需要时保留
  LICENSE.md
  OpenXLSX.VERSION.txt
```

`ThirdParty` 放在插件根目录下，保持和 `Plugins/Puerts/ThirdParty` 一致；`Source` 目录只放 UE 模块代码。

## 3. 核心类型

### 3.1 `UEnhancedDataTable`

`UEnhancedDataTable` 继承自 `UDataTable`。

职责：

- 保留原生 DataTable 的运行时访问方式。
- 保存 Excel 源文件路径。
- 保存可选 Sheet 名称。
- 保存有效数据区域配置。

当前字段：

```cpp
UPROPERTY(EditAnywhere, Category = "Excel Import", meta = (FilePathFilter = "Excel files (*.xlsx;*.xls)|*.xlsx;*.xls|All files (*.*)|*.*"))
FFilePath ExcelSourceFile;

UPROPERTY(EditAnywhere, Category = "Excel Import")
FName ExcelSheetName;

UPROPERTY(EditAnywhere, Category = "Excel Import", meta = (ClampMin = "1"))
int32 ExcelFirstDataRow = 2;

UPROPERTY(EditAnywhere, Category = "Excel Import", meta = (ClampMin = "1"))
int32 ExcelFirstColumn = 1;
```

`ExcelSheetName` 为空时，首版读取默认 Sheet。后续需要指定 Sheet 时，再使用该字段做精确选择。
行列号使用 Excel 习惯的 1-based 编号。字段名行固定为 `ExcelFirstDataRow - 1`。`ExcelFirstColumn` 是序号列，后一列固定作为 `RowName`，属性字段从再后一列开始。

### 3.2 `UEnhancedDataTableFactory`

职责：

- 复用 `UDataTableFactory` 的 `RowStruct` 选择流程。
- 重写 `MakeNewDataTable()`，创建 `UEnhancedDataTable`。
- 让 Content Browser 可以新建 `Enhanced Data Table` 资产。

### 3.3 `UAssetDefinition_EnhancedDataTable`

职责：

- `GetAssetClass()` 返回 `UEnhancedDataTable::StaticClass()`。
- `OpenAssets()` 中创建 `FEnhancedDataTableEditor`。
- 不调用 `FDataTableEditorModule::CreateDataTableEditor()`。

### 3.4 `FEnhancedDataTableEditor`

职责：

- 展示 DataTable 行列数据。
- 提供搜索过滤。
- 提供基础行操作按钮。
- 提供 Excel 文件路径输入、文件选择按钮和导入按钮。
- 调用 `EnhancedDataTableUtils` 完成 `.xlsx` 解析和 DataTable 写入。
- 导入成功后刷新缓存数据和视图。

复刻边界：

- 复刻原生 DataTable 编辑器的核心表格显示和行操作。
- 不复刻首版不需要的 Diff、`CompositeDataTable` 专用逻辑和完整菜单导出能力。
- 后续 UE 升级时，以引擎 `FDataTableEditor` 为对照源同步必要改动。

### 3.5 `EnhancedDataTableUtils`

`EnhancedDataTableUtils` 是导入流程的业务边界。Editor 只负责交互，Utils 负责读取、校验和写入。

建议数据结构：

```cpp
struct FEnhancedDataTableExcelSheet
{
	TArray<FString> Headers;
	TArray<TArray<FString>> Rows;
};

struct FEnhancedDataTableImportResult
{
	TArray<FString> Errors;
	TArray<FString> Warnings;
	int32 ImportedRowCount = 0;
};
```

建议接口：

```cpp
class FEnhancedDataTableUtils
{
public:
	static bool ReadXlsx(
		const FString& FilePath,
		const FName SheetName,
		FEnhancedDataTableExcelSheet& OutSheet,
		TArray<FString>& OutErrors);

	static bool ImportXlsxToDataTable(
		UEnhancedDataTable* DataTable,
		FEnhancedDataTableImportResult& OutResult);
};
```

职责拆分：

- `ReadXlsx()` 只负责读取 `.xlsx` 并生成二维字符串表。
- `ImportXlsxToDataTable()` 负责校验字段、创建行数据、写入 `UEnhancedDataTable`。
- `FEnhancedDataTableEditor::ImportExcel()` 只负责 UI 校验、调用 Utils、展示结果、刷新视图。

## 4. 功能实现方案

### 4.1 OpenXLSX 集成

集成原则：

- `OpenXLSX` 只服务 Editor 导入流程。
- `EnhancedDataTable` 只在 `Target.bBuildEditor` 下依赖 `OpenXLSX`。
- 不把 `.xlsx` 解析能力带入游戏运行时包。
- `.xls` 不在首版支持范围内，导入时提示用户另存为 `.xlsx`。
- 先按 `OpenXLSX` 官方依赖完整导入 `pugixml`、`miniz`、必要时的 `nowide`，完成 Editor 构建验证。
- 编译验证通过后执行 ThirdParty 裁剪，只保留导入功能需要参与编译或需要随库分发的文件。
- 被裁剪掉的内容包括 `Examples`、`Tests`、`Benchmarks`、`Documentation`、无关脚本和 CMake 辅助文件；如后续升级需要，重新从记录的来源仓库和 commit 拉取。
- 保留第三方库许可证、版本来源记录和必要的构建适配文件，方便升级和审计。

首版读取能力：

- 打开 `.xlsx` 文件。
- 默认读取第一个 Sheet。
- 后续再根据 `UEnhancedDataTable::ExcelSheetName` 读取指定 Sheet。
- 将单元格统一转换成 `FString`。
- 不处理样式。
- 公式单元格优先读取缓存值；没有缓存值时记录错误。
- 合并单元格不做特殊展开，按实际单元格值读取。

### 4.2 Excel 格式约定

首版表格格式：

- 有效数据区域通过 `UEnhancedDataTable` 配置。
- `ExcelFirstDataRow` 指定第一条 DataTable Row 所在行，默认第 2 行。
- 字段名行固定为 `ExcelFirstDataRow - 1`。
- `ExcelFirstColumn` 指定序号列，默认第 1 列。
- `ExcelFirstColumn + 1` 固定作为 `RowName` 列。
- 序号列和 `RowName` 列不依赖 Header 文本，无论字段名行写什么，都按序号和 `RowName` 处理。
- Header 从 `ExcelFirstColumn + 2` 开始向右读取属性字段名。
- Header 遇到第一个空字段名单元格时停止读取字段。
- `ExcelFirstDataRow` 开始每一行是一条 DataTable Row。
- 数据行从 `ExcelFirstColumn` 开始读取序号、`RowName` 和属性字段值。
- 序号列首版只用于人工查看和空行判断，不写入 DataTable。
- 读到空行时认为有效数据已经结束，后续行不再读取。
- `RowName` 为空时报错。
- `RowName` 重复时报错。
- 字段名默认精确匹配 `UScriptStruct` 属性名。
- 不存在的字段默认报错。
- 空单元格按空字符串导入，是否保留默认值后续再加选项。

字段导入规则：

- 不为每种 UE 类型手写转换器。
- 根据 `RowStruct` 找到对应 `FProperty`。
- 使用 UE 属性系统导入文本：

```cpp
Property->ImportText_Direct(
	*CellText,
	Property->ContainerPtrToValuePtr<void>(RowData),
	DataTable,
	PPF_None);
```

这样 `int`、`float`、`bool`、`enum`、`FName`、`FString`、`FGameplayTag`、`FSoftObjectPath` 等类型可以沿用 UE DataTable 的文本导入规则。

### 4.3 写入 DataTable 流程

`ImportXlsxToDataTable()` 的写入流程：

1. 检查 `DataTable` 非空。
2. 检查 `DataTable->GetRowStruct()` 有效。
3. 校验 Excel 有效数据区域配置：
   - `ExcelFirstDataRow` 必须大于 1。
   - `ExcelFirstColumn` 必须大于 0。
   - `ExcelFirstColumn` 和后一列 `RowName` 必须在 `.xlsx` 列数上限内。
4. 调用 `ReadXlsx()` 从指定 Sheet 的 `ExcelFirstDataRow - 1`、`ExcelFirstDataRow` 和 `ExcelFirstColumn` 开始读取。
5. 校验 Header：
   - 序号列和 `RowName` 列不校验 Header 文本。
   - 从 `ExcelFirstColumn + 2` 开始，每个字段名能匹配 `RowStruct` 属性。
   - 遇到空字段名后停止读取后续字段。
6. 校验所有行：
   - `RowName` 非空。
   - `RowName` 不重复。
   - 每个单元格能被目标 `FProperty` 导入。
   - 遇到空行后停止读取后续数据行。
7. 所有校验通过后才开始写入。
8. 使用 `FScopedTransaction` 支持 Undo。
9. 调用 `DataTable->Modify()`。
10. 调用 `FDataTableEditorUtils::BroadcastPreChange(DataTable, RowList)`。
11. 清空旧行。
12. 添加新行并导入属性。
13. 对每行调用 `DataTable->HandleDataTableChanged(RowName)`。
14. 调用 `DataTable->MarkPackageDirty()`。
15. 调用 `FDataTableEditorUtils::BroadcastPostChange(DataTable, RowList)`。

首版写入策略：

- 使用“清空旧行后导入”。
- 任意错误则整体不写入。
- 后续再增加增量覆盖、保留旧值、忽略额外列等选项。

### 4.4 Editor 调用流程

`FEnhancedDataTableEditor::ImportExcel()` 后续改为：

```cpp
void FEnhancedDataTableEditor::ImportExcel()
{
	// 1. UI 层校验 DataTable、RowStruct、路径、扩展名。
	// 2. 调用 FEnhancedDataTableUtils::ImportXlsxToDataTable()。
	// 3. 展示 Errors / Warnings。
	// 4. 成功后 RefreshCachedDataTable()。
}
```

UI 层只做用户交互，不直接解析 `.xlsx`，也不直接写 RowMap。

## 5. 实现步骤

1. 下载并导入 `OpenXLSX` 到 `Plugins/DMToolBox/ThirdParty/OpenXLSX/`。
2. 下载并导入 `OpenXLSX` 编译所需依赖，首版按 `pugixml`、`miniz`、必要时 `nowide` 处理。
3. 移除所有第三方库内部 `.git` 目录，并记录来源仓库、tag 或 commit。
4. 编写或调整 `EnhancedDataTable.Build.cs`，只在 `Target.bBuildEditor` 下暴露 `OpenXLSX` 和依赖库 include / compile path。
5. 编译 Editor Target，确认 `OpenXLSX` 和依赖库可以在 UE 模块中链接通过。
6. 裁剪 ThirdParty：只保留 `OpenXLSX` 读取 `.xlsx` 需要的 headers、sources、依赖源码、license 和 version 文件。
7. 新增 `EnhancedDataTableUtils.h/.cpp`。
8. 实现 `ReadXlsx()`，读取默认 Sheet 到 `FEnhancedDataTableExcelSheet`。
9. 在 `ReadXlsx()` 中处理文件不存在、扩展名不支持、文件损坏、Sheet 不存在等错误。
10. 实现 Header 校验：`RowName`、重复列、未知字段。
11. 实现 Row 校验：空行结束、空 `RowName`、重复 `RowName`、属性文本导入失败。
12. 实现 `ImportXlsxToDataTable()` 的事务、清空旧行、写入新行和变更广播。
13. 修改 `FEnhancedDataTableEditor::ImportExcel()` 调用 Utils。
14. 使用最小测试表验证创建、读取、写入和错误阻断。

## 6. 使用规范

资产使用：

- 需要 Excel 导入能力时创建 `Enhanced Data Table`。
- 不需要 Excel 导入能力时继续创建普通 `Data Table`。
- `UEnhancedDataTable` 的 `RowStruct` 仍然决定最终字段类型和导入规则。

Excel 文件约定：

- 文件格式使用 `.xlsx`。
- `ExcelFirstDataRow - 1` 指定的行写属性名。
- `ExcelFirstDataRow` 开始写有效数据。
- `ExcelFirstColumn` 指定序号列，下一列固定为 `RowName`。
- 序号列和 `RowName` 列的 Header 文本会被忽略。
- Header 从 `ExcelFirstColumn + 2` 开始连续写属性名，遇到空字段名单元格结束。
- 数据行从 `ExcelFirstColumn` 开始连续填写序号、`RowName` 和字段值，遇到空行结束。
- `RowName` 必须稳定，避免导入后引用失效。
- 单元格文本应符合 UE DataTable 的文本导入格式。

Sheet 约定：

- 首版默认读取第一个 Sheet。
- 需要指定 Sheet 时，后续通过 `UEnhancedDataTable::ExcelSheetName` 接入。

## 7. 验证标准

创建和编辑器验证：

- Content Browser 可以创建 `Enhanced Data Table`。
- 创建时可以选择 `RowStruct`。
- 双击 `UEnhancedDataTable` 打开 `FEnhancedDataTableEditor`。
- 普通 `UDataTable` 仍打开原生编辑器。
- Excel 路径能保存到资产。

`.xlsx` 读取验证：

- 能读取默认 Sheet。
- 能读取中文、英文、数字、空单元格。
- `.xls` 文件会明确提示不支持。
- 文件不存在、文件损坏、Sheet 不存在时能给出错误。
- 公式单元格没有缓存值时会记录错误。

DataTable 写入验证：

- 最小字符串表导入成功。
- `int`、`float`、`bool`、`enum` 字段导入成功。
- `FGameplayTag` 字段导入成功。
- 重复 `RowName` 会阻止导入。
- 缺失 `RowName` 会阻止导入。
- 字段名不匹配会阻止导入。
- 任意错误时旧 DataTable 不被修改。
- 成功导入后支持 Undo。

## 8. 后续扩展点

后续可以按需增加：

- 根据 `UEnhancedDataTable::ExcelSheetName` 读取指定 Sheet。
- 增量覆盖而不是清空后导入。
- 保留旧值或默认值的空单元格策略。
- 忽略额外列的导入选项。
- 字段别名或显示名映射。
- 导入前差异预览。
- 更完整的导入报告面板。

这些能力不进入当前第一版完成标准。

## 9. 当前进度和剩余事项

已完成：

- `UEnhancedDataTable`
- `UEnhancedDataTableFactory`
- `UAssetDefinition_EnhancedDataTable`
- `FEnhancedDataTableEditor`
- Excel 路径持久化
- 工具栏基础行操作按钮
- `Import` 按钮的基础校验入口

下一步：

- 导入 `OpenXLSX`。
- 新增 `EnhancedDataTableUtils`。
- 实现 `.xlsx` 读取。
- 实现 `.xlsx` 到 `UEnhancedDataTable` 的写入。
- 补齐最小验证用 `.xlsx` 测试数据。

第一版完成条件：

- `UEnhancedDataTable` 能创建、打开并保存 Excel 路径。
- Editor 构建能成功链接 `OpenXLSX`。
- `Import` 能读取 `.xlsx` 默认 Sheet 并写入 DataTable。
- 导入失败时能展示明确错误，并且不修改旧 DataTable。
- 普通 `UDataTable` 行为不受影响。
