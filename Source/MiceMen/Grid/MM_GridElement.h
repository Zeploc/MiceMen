// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IntVector2D.h"
#include "MM_GridElement.generated.h"


class AMM_ColumnControl;
class AMM_GridManager;

/**
 * Base class for one grid item
 */
UCLASS()
class MICEMEN_API AMM_GridElement : public AActor
{
	GENERATED_BODY()

public:
	AMM_GridElement();

	/** Stores initial information for the grid element */
	virtual void SetupGridInfo(class AMM_GridManager* _GridManager, FIntVector2D _GridCoordinates);

	/** Changes the grid position, updating the column this element is linked to */
	virtual void UpdateGridPosition(FIntVector2D _NewGridCoordiantes);

	/** Called when grid object is cleaning up elements */
	virtual void CleanUp();

	UFUNCTION(BlueprintPure)
	FIntVector2D GetCoordinates() {
		return Coordinates;
	}

	UFUNCTION(BlueprintPure)
		AMM_GridManager* GetGridManager();

protected:


public:

protected:
	/** The current grid coordinates of this element */
	UPROPERTY(BlueprintReadOnly)
		FIntVector2D Coordinates;

	/** The column this element is currently linked to */
	UPROPERTY(BlueprintReadOnly)
	AMM_ColumnControl* CurrentColumn;

	UPROPERTY(BlueprintReadOnly)
		AMM_GridManager* GridManager;

};
