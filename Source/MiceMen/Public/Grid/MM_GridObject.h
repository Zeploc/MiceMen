// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "IntVector2D.h"
#include "Base/MM_GridEnums.h"
#include "MM_GridObject.generated.h"

class AMM_GridElement;

/**
 * The main control for the grids array and elements, with some helpers for coordinates and free slots
 */
UCLASS()
class MICEMEN_API UMM_GridObject : public UObject
{
	GENERATED_BODY()

#pragma region Grid

public:
	/** Setups up initial grid sizes */
	void SetupGrid(const FIntVector2D& InGridSize);

	/** Empties grid objects and elements */
	void CleanUp();

#pragma endregion

#pragma region Elements

public:
	/** Returns a grid element at given coordinates, returns null if no element */
	UFUNCTION(BlueprintPure)
	AMM_GridElement* GetGridElement(const FIntVector2D& Coord) const;

	/** Sets a grid element in the grid array */
	bool SetGridElement(const FIntVector2D& Coord, AMM_GridElement* GridElement);

	bool MoveGridElement(const FIntVector2D& NewCoord, AMM_GridElement* GridElement);

	/** Moves the column up or down by 1 and returns the last element */
	AMM_GridElement* MoveColumnElements(int Column, EDirection Direction);

#pragma endregion

#pragma region Coordinates

public:
	/** Checks if the given coordinates are valid */
	UFUNCTION(BlueprintPure)
	bool IsValidCoord(const FIntVector2D& Coord) const;

	/** Gets random coordinate in grid, optional search for free slot */
	UFUNCTION(BlueprintPure)
	FIntVector2D GetRandomGridCoord(bool bFreeSlot = true) const;

	/** Gets random coordinate in grid based on a given column (x axis) range */
	UFUNCTION(BlueprintPure)
	FIntVector2D GetRandomGridCoordInColumnRange(int MinX, int MaxX, bool bFreeSlot = true) const;

	/**
	* Gets random coordinate in range either as a free slot or any.
	* @param MinX minimum along the X axis
	* @param MaxX maximum along the X axis
	* @param MinY minimum along the Y axis
	* @param MaxY maximum along the Y axis
	* @param bFreeSlot if a free slot needs to be found, default true
	* @return random coordinate in range, (-1, -1) if failed
	*/
	UFUNCTION(BlueprintPure)
	FIntVector2D GetRandomGridCoordInRange(int MinX, int MaxX, int MinY, int MaxY, bool bFreeSlot = true) const;

	/** Returns true if the given coordinate is with in the X and Y range on the grid */
	UFUNCTION(BlueprintPure)
	static bool IsCoordInRange(const FIntVector2D& Coord, int MinX, int MaxX, int MinY, int MaxY);

protected:
	/** Converts a coordinate to the index in the grid array */
	int CoordToIndex(int X, int Y) const;

#pragma endregion

#pragma region Free Slots

public:
	/** Will check one slot in a given direction, and return true if its free, setting CurrentPosition */
	UFUNCTION(BlueprintPure)
	bool FindFreeSlotInDirection(FIntVector2D& CurrentPosition, const FIntVector2D& Direction) const;

	/** Will check for the lowest possible free slot below without passing through a taken element, and return true if its free, setting CurrentPosition */
	UFUNCTION(BlueprintPure)
	bool FindFreeSlotBelow(FIntVector2D& CurrentPosition) const;

	/** Will check one slot ahead horizontally, based on the given direction, and return true if its free, setting CurrentPosition */
	UFUNCTION(BlueprintPure)
	bool FindFreeSlotAhead(FIntVector2D& CurrentPosition, EDirection Direction) const;

	/** Lists the active free slots in the grid*/
	UFUNCTION(BlueprintPure)
	TArray<FIntVector2D> GetFreeSlots() const { return FreeSlots; }

#pragma endregion

//-------------------------------------------------------

#pragma region Grid Variables

protected:
	/**
	* One dimensional array for two dimensional grid
	*/
	TArray<AMM_GridElement*> Grid;

	/** The size of the grid */
	UPROPERTY(BlueprintReadOnly)
	FIntVector2D GridSize;

#pragma endregion

#pragma region Free Slot Variables

protected:
	/**
	* List of available free slots updated by the grid.
	* Removes the need to iterate all slots
	*/
	TArray<FIntVector2D> FreeSlots;

#pragma endregion
};
