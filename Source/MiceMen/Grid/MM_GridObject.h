// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "IntVector2D.h"
#include "MM_GridObject.generated.h"

class AMM_GridElement;

/**
 * 
 */
UCLASS()
class MICEMEN_API UMM_GridObject : public UObject
{
	GENERATED_BODY()

public:

	/** Setups up initial grid sizes */
	void SetupGrid(FIntVector2D _GridSize);
	/** Empties grid objects and elements */
	void CleanUp();

	/** Returns a grid element at given coordinates, returns null if no element */
	AMM_GridElement* GetGridElement(const FIntVector2D& _Coord) const;

	/** Checks if the given coordinates are valid */
	bool IsValidCoord(const FIntVector2D& _Coord) const;

	/** Converts a coordiante to the index in the grid array */
	int CoordToIndex(int _X, int _Y) const;
	/** Sets a grid element in the grid array */
	bool SetGridElement(const FIntVector2D& _Coord, class AMM_GridElement* _GridElement);

	TArray<FIntVector2D> GetFreeSlots() { return FreeSlots; }

	/** Moves the column up or down by 1 and returns the last element */
	AMM_GridElement* MoveColumnElements(int _Column, int _Direction);

	/** Gets random coordinate in grid, optional search for free slot */
	FIntVector2D GetRandomGridCoord(bool _bFreeSlot = true);
	/** Gets random coordinate in grid based on a given column (x axis) range */
	FIntVector2D GetRandomGridCoordInColumnRange(int _MinX, int _MaxX, bool _bFreeSlot = true);

	/**
	* Gets random coordinate in range either as a free slot or any. 
	* @param _MinX minimum along the X axis
	* @param _MaxX maximum along the X axis
	* @param _MinY minimum along the Y axis
	* @param _MaxY maximum along the Y axis
	* @param _bFreeSlot if a free slot needs to be found, default true
	* @return random coordinate in range, (-1, -1) if failed
	*/
	FIntVector2D GetRandomGridCoordInRange(int _MinX, int _MaxX, int _MinY, int _MaxY, bool _bFreeSlot = true) const;

	/** Returns true if the given coordinate is with in the X and Y range on the grid */
	bool IsCoordInRange(const FIntVector2D& _Coord, int _MinX, int _MaxX, int _MinY, int _MaxY) const;

	/** Will check one slot in a given direction, and return true if its free, setting CurrentPosition */
	bool FindFreeSlotInDirection(FIntVector2D& _CurrentPosition, const FIntVector2D _Direction);
	/** Will check for the lowest possible free slot below without passing through a taken element, and return true if its free, setting CurrentPosition */
	bool FindFreeSlotBelow(FIntVector2D& _CurrentPosition);
	/** Will check one slot ahead horizontally, based on the given direction, and return true if its free, setting CurrentPosition */
	bool FindFreeSlotAhead(FIntVector2D& _CurrentPosition, int _Direction);

	// TODO Move to mouse for dynamic movement
	/** Gets a valid path for an element, giving the horizontal direction to move towards */
	TArray<FIntVector2D> GetValidPath(FIntVector2D _StartingPosition, int _iHorizontalDirection = 1);

protected:
	void OutputColumnDisplace(AMM_GridElement* NextElement, AMM_GridElement* CurrentElement, FIntVector2D& CurrentSlot);


public:


protected:

		/**
		 * One dimensional array for two dimensional grid
		 */
		TArray<AMM_GridElement*> Grid;

		/**
		 * Coord to index position.
		 * Used as lookup, updated when a change in the grid occurs
		 * Could store key as array index, but storing as coords
		 * removes a need to run an operation to calculate from index to coord
		 */

		/**
		 * List of available free slots updated by the grid.
		 * Removes the need to iterate all slots
		 */
		TArray<FIntVector2D> FreeSlots;

		/** The size of the grid */
		FIntVector2D GridSize;

};
