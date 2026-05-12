// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_EDITOR

#include "CoreMinimal.h"

THIRD_PARTY_INCLUDES_START
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4005)
#pragma warning(disable : 4101)
#endif
#include "../../ThirdParty/OpenXLSX/external/miniz/miniz.c"
#include "../../ThirdParty/OpenXLSX/external/miniz/miniz_tdef.c"
#include "../../ThirdParty/OpenXLSX/external/miniz/miniz_tinfl.c"
#include "../../ThirdParty/OpenXLSX/external/miniz/miniz_zip.c"
#include "../../ThirdParty/OpenXLSX/external/pugixml/src/pugixml.cpp"
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLCell.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLCellIterator.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLCellRange.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLCellReference.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLCellValue.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLColor.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLColumn.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLComments.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLContentTypes.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLDateTime.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLDocument.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLDrawing.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLFormula.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLMergeCells.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLProperties.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLRelationships.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLRow.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLRowData.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLSharedStrings.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLSheet.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLStyles.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLTables.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLWorkbook.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLXmlData.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLXmlFile.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLXmlParser.cpp"
#include "../../ThirdParty/OpenXLSX/OpenXLSX/sources/XLZipArchive.cpp"
#ifdef _MSC_VER
#pragma warning(pop)
#endif
THIRD_PARTY_INCLUDES_END

#endif // WITH_EDITOR
