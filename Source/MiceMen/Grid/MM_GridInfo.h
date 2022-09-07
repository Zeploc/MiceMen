// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MM_GridInfo.generated.h"

/**
 * Information for one grid item
 */
UCLASS(Blueprintable)
class MICEMEN_API UMM_GridInfo : public UObject
{
	GENERATED_BODY()

public:
	UMM_GridInfo();

	void SetupGridInfo(FIntVector _GridCoordinates);

	void SetGridBlock(class AMM_GridBlock* _Block);

protected:


public:
	UFUNCTION(BlueprintPure)
		bool IsEmpty();

protected:
	UPROPERTY(BlueprintReadOnly)
		FIntVector Coordinates;

	UPROPERTY(BlueprintReadOnly)
		class AMM_Mouse* Mouse;
	UPROPERTY(BlueprintReadOnly)
		class AMM_GridBlock* Block;

};
