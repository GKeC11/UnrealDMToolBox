#pragma once

#include "DMToolBox/Gameplay/Data/DMLevelInitializationSetting.h"

#include "DMWorldSetting.generated.h"

UCLASS()
class ADMWorldSetting : public AWorldSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DMToolBox")
	UDMLevelInitializationSetting* LevelInitializationSetting;
};
