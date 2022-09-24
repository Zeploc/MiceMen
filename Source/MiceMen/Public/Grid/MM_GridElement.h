// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IntVector2D.h"
#include "MM_GridElement.generated.h"

class AMM_ColumnControl;
class AMM_GridManager;
class AMM_GameMode;

/**
 * Base class for one grid item
 */
UCLASS()
class MICEMEN_API AMM_GridElement : public AActor
{
	GENERATED_BODY()

public:
	AMM_GridElement();

#pragma region Grid

public:
	/** Stores initial information for the grid element */
	virtual void SetupGridVariables(AMM_GridManager* _GridManager, AMM_GameMode* _MMGameMode, const FIntVector2D& _GridCoordinates);

	/** Changes the grid position, updating the column this element is linked to */
	virtual void UpdateGridPosition(const FIntVector2D& _NewGridCoordiantes);

	UFUNCTION(BlueprintPure)
	FIntVector2D GetCoordinates() const { return Coordinates; }

	UFUNCTION(BlueprintPure)
	AMM_GridManager* GetGridManager();

#pragma endregion

#pragma region Cleanup

public:
	/** Called when grid object is cleaning up elements */
	virtual void CleanUp();

#pragma endregion

//-------------------------------------------------------

#pragma region Grid Variables

protected:
	/** The current grid coordinates of this element */
	UPROPERTY(BlueprintReadOnly)
	FIntVector2D Coordinates;
	
	UPROPERTY(BlueprintReadOnly, BlueprintGetter=GetGridManager)
	AMM_GridManager* GridManager;

	UPROPERTY(BlueprintReadOnly)
	AMM_GameMode* MMGameMode;

	/** The column this element is currently linked to */
	UPROPERTY(BlueprintReadOnly)
	AMM_ColumnControl* CurrentColumn;

#pragma endregion

};
