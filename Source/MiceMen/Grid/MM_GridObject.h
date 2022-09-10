// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "IntVector2D.h"
#include "MM_GridObject.generated.h"

/**
 * 
 */
UCLASS()
class MICEMEN_API UMM_GridObject : public UObject
{
	GENERATED_BODY()

public:
	//TODO: Add function headers

	void SetupGrid(FIntVector2D _GridSize);
	void CleanUp();

	class AMM_GridElement* GetGridElement(FIntVector2D _Coord);

	int CoordToIndex(int _X, int _Y);
	bool SetGridElement(FIntVector2D _Coord, class AMM_GridElement* _GridElement);

	TArray<FIntVector2D> GetFreeSlots() { return FreeSlots; }

	void RegenerateFreeSlots();

	FIntVector2D GetRandomGridCoord(bool _bFreeSlot = true);
	FIntVector2D GetRandomGridCoordInColumnRange(int _MinX, int _MaxX, bool _bFreeSlot = true);
	FIntVector2D GetRandomGridCoordInRange(int _MinX, int _MaxX, int _MinY, int _MaxY, bool _bFreeSlot = true);

	bool FindFreeSlotInDirection(FIntVector2D& _CurrentPosition, FIntVector2D _Direction);
	bool FindFreeSlotBelow(FIntVector2D& _CurrentPosition);
	bool FindFreeSlotAhead(FIntVector2D& _CurrentPosition, int _Direction);

	TArray<FIntVector2D> GetValidPath(FIntVector2D _StartingPosition, int _iHorizontalDirection = 1);

protected:
	FIntVector2D GridSize;


public:


protected:

private:
		/**
		 * One dimensional array for two dimensional grid
		 */
		TArray<class AMM_GridElement*> Grid;


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



		///**
		// * Generated from FreeSlots to have array to remove keys generation.
		// * This is to remove the GenerateKeys() from the map, created an extra iteration
		// * This is based on CPU being prioritised over memory
		// */
		//TArray<FIntVector2D> FreeSlotKeys;
};
