// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MM_FunctionLibrary.generated.h"

class UMM_Singleton;

/**
 * A collection of static functions used for the project
 */
UCLASS()
class MICEMEN_API UMM_FunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/** Retrieve the MM singleton */
	UFUNCTION(BlueprintPure)
	static UMM_Singleton* GetMMSingleton();

protected:
	/** Static pointer to the singleton so only one cast is needed, use GetMMSingleton() to retreive */
	static UMM_Singleton* MMSingleton;
};
