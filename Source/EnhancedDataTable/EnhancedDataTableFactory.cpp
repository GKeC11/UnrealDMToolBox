// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnhancedDataTableFactory.h"

#if WITH_EDITOR

#include "EnhancedDataTable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EnhancedDataTableFactory)

UEnhancedDataTableFactory::UEnhancedDataTableFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UEnhancedDataTable::StaticClass();
}

UDataTable* UEnhancedDataTableFactory::MakeNewDataTable(UObject* InParent, FName Name, EObjectFlags Flags)
{
	return NewObject<UEnhancedDataTable>(InParent, Name, Flags);
}

#endif // WITH_EDITOR
