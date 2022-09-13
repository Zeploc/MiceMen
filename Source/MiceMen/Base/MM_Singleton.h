// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MM_Singleton.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class MICEMEN_API UMM_Singleton : public UObject
{
	GENERATED_BODY()

public:
	UMM_Singleton(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TMap<int, FLinearColor> TeamColours;
	
};
