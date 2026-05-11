#pragma once

#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "InputTriggers.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "DMUILibrary.generated.h"

class UInputAction;
class UUserWidget;

DECLARE_DYNAMIC_DELEGATE_FiveParams(FDMUIEnhancedInputActionDelegate,
	FInputActionValue, ActionValue,
	float, ElapsedTime,
	float, TriggeredTime,
	const UInputAction*, SourceAction,
	ETriggerEvent, TriggerEvent);

UCLASS()
class DMTOOLBOX_API UDMUILibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "DMToolBox|UI", meta = (WorldContext = "WorldContextObject"))
	static bool CreateWidgetByTagToLayer(UObject* WorldContextObject, FGameplayTag InWidgetTag, FGameplayTag InLayerTag);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|UI", meta = (WorldContext = "WorldContextObject"))
	static bool RemoveWidgetFromLayer(UObject* WorldContextObject, UCommonActivatableWidget* InWidget, FGameplayTag InLayerTag);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|UI", meta = (WorldContext = "WorldContextObject"))
	static bool RemoveWidgetByTagFromLayer(UObject* WorldContextObject, FGameplayTag InWidgetTag, FGameplayTag InLayerTag);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|UI|Input")
	static int32 BindEnhancedInputActionForWidget(UUserWidget* InWidget, UInputAction* InInputAction, ETriggerEvent InTriggerEvent, FDMUIEnhancedInputActionDelegate InCallback);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|UI|Input")
	static bool UnbindEnhancedInputActionForWidget(UUserWidget* InWidget, int32 InBindingHandle);

	static void CreateWidgetToLayer(APlayerController* InPlayerController, TSubclassOf<UCommonActivatableWidget> InWidgetClass, FGameplayTag InLayerTag);
};
