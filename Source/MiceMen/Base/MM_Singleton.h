// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MM_Singleton.generated.h"

/**
 * A global class used for storing variables that can be accessed in editor as well as runtime
 */
UCLASS(Blueprintable, BlueprintType)
class MICEMEN_API UMM_Singleton : public UObject
{
	GENERATED_BODY()

public:
	UMM_Singleton(const FObjectInitializer& ObjectInitializer);

	/** The colours for the teams */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<int, FLinearColor> TeamColours;
	
};
