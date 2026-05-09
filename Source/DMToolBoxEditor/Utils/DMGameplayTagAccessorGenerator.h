#pragma once

#include "CoreMinimal.h"

class FDMGameplayTagAccessorGenerator
{
public:
	static bool Generate(FText& OutMessage);

private:
	struct FCollectedTag
	{
		FName TagName;
		FString DevComment;
		FString SourceTable;
	};

	static bool CollectTags(TArray<FCollectedTag>& OutTags, FText& OutMessage);
	static bool WriteHeader(const TArray<FCollectedTag>& Tags, FText& OutMessage);
	static FString BuildGeneratedBlock(const TArray<FCollectedTag>& Tags);
	static FString BuildAccessorName(FName TagName, TSet<FString>& UsedNames);
	static FString SanitizeComment(const FString& Comment);
	static bool ReplaceOrAppendGeneratedBlock(FString& FileContent, const FString& GeneratedBlock);
};
