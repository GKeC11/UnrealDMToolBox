#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "DMToolBox/Gameplay/Data/DMLevelInitializationSetting.h"

#include "DMSystemLibrary.generated.h"

class UDMGameInstance;
class APlayerController;
class UEnhancedInputComponent;
class UObject;
class UUserWidget;
class UWorld;

UCLASS()
class UDMSystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static UDMGameInstance* ResolveDMGameInstance();
	static UWorld* ResolveWorldFromContext(const UObject* WorldContextObject);
	static int32 ResolveClientIndex(const UWorld* World);
	static APlayerController* ResolveOwningPlayerFromWidget(UUserWidget* InWidget);
	static UEnhancedInputComponent* ResolveEnhancedInputComponentFromWidget(UUserWidget* InWidget);

	static UDMLevelInitializationSetting* GetLevelInitializationSetting(UWorld* InWorld);
};
