// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#if WITH_EDITOR

#include "Factories/DataTableFactory.h"

#include "EnhancedDataTableFactory.generated.h"

/**
 * Factory for creating Enhanced DataTable assets while reusing the native DataTable row-struct picker.
 */
UCLASS(hidecategories = Object)
class ENHANCEDDATATABLE_API UEnhancedDataTableFactory : public UDataTableFactory
{
	GENERATED_BODY()

public:
	UEnhancedDataTableFactory(const FObjectInitializer& ObjectInitializer);

protected:
	virtual UDataTable* MakeNewDataTable(UObject* InParent, FName Name, EObjectFlags Flags) override;
};

#endif // WITH_EDITOR
