// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IntVector2D.h"
#include "MM_GridElement.generated.h"

/**
 * Base class for one grid item
 */
UCLASS()
class MICEMEN_API AMM_GridElement : public AActor
{
	GENERATED_BODY()

public:
	AMM_GridElement();

	void SetupGridInfo(class AMM_GridManager* _GridManager, FIntVector2D _GridCoordinates);

	void UpdateGridPosition(FIntVector2D _NewGridCoordiantes);

	void CleanUp();

	UFUNCTION(BlueprintPure)
	FIntVector2D GetCoordinates() {
		return Coordinates;
	}

protected:


public:

protected:
	UPROPERTY(BlueprintReadOnly)
		FIntVector2D Coordinates;

	UPROPERTY(BlueprintReadOnly)
	class AMM_ColumnControl* CurrentColumn;

	UPROPERTY(BlueprintReadOnly)
		class AMM_GridManager* GridManager;

};
