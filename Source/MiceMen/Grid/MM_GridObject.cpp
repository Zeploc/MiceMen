// Copyright Alex Coultas, Mice Men Example Project


#include "Grid/MM_GridObject.h"

#include "MM_GridElement.h"

void UMM_GridObject::SetupGrid(FIntVector2D _GridSize)
{
	GridSize = _GridSize;
	Grid.SetNumZeroed(GridSize.X * GridSize.Y);

}

void UMM_GridObject::CleanUp()
{
	for (AMM_GridElement* GridElement : Grid)
	{
		if (GridElement)
			GridElement->CleanUp();
	}
	Grid.Empty();

	FreeSlots.Empty();
	//FreeSlotKeys.Empty();
}

// ################################ Grid Management ################################

AMM_GridElement* UMM_GridObject::GetGridElement(FIntVector2D _Coord)
{
	// Not in grid range
	if (_Coord.X >= GridSize.X || _Coord.Y >= GridSize.Y)
		return nullptr;

	// Check grid size is correct 
	if (Grid.Num() != GridSize.X * GridSize.Y)
		return nullptr;

	return Grid[CoordToIndex(_Coord.X, _Coord.Y)];
}

int UMM_GridObject::CoordToIndex(int _X, int _Y)
{
	return _X * GridSize.Y + _Y;
}

bool UMM_GridObject::SetGridElement(FIntVector2D _Coord, class AMM_GridElement* _GridElement)
{
	// Not in grid range
	if (_Coord.X >= GridSize.X || _Coord.Y >= GridSize.Y)
		return false;

	// Check grid size is correct 
	if (Grid.Num() != GridSize.X * GridSize.Y)
		return false;

	Grid[CoordToIndex(_Coord.X, _Coord.Y)] = _GridElement;

	// Update grid element if valid/not empty and update FreeSlots map
	if (_GridElement)
	{
		// Only update if a change in coordinates occurred
		if (_GridElement->GetCoordinates() != _Coord)
			_GridElement->UpdateGridPosition(_Coord);

		// Update Free slots to no longer have this element
		FreeSlots.Remove(_Coord);
	}
	else
	{
		// No element (nullptr), add this coord to free slots
		FreeSlots.Add(_Coord);
	}

	return true;
}

void UMM_GridObject::RegenerateFreeSlots()
{
	// Update FreeSlots array
	//FreeSlots.GenerateKeyArray(FreeSlotKeys);

}

// ################################ Grid Helpers ################################


FIntVector2D UMM_GridObject::GetRandomGridCoord(bool _bFreeSlot /*= true*/)
{
	return GetRandomGridCoordInRange(0, GridSize.X, 0, GridSize.Y, _bFreeSlot);
}

FIntVector2D UMM_GridObject::GetRandomGridCoordInColumnRange(int _MinX, int _MaxX, bool _bFreeSlot /*= true*/)
{
	return GetRandomGridCoordInRange(_MinX, _MaxX, 0, GridSize.Y, _bFreeSlot);
}

FIntVector2D UMM_GridObject::GetRandomGridCoordInRange(int _MinX, int _MaxX, int _MinY, int _MaxY, bool _bFreeSlot /*= true*/)
{
	// Clamp ranges
	int ClampedMinX = FMath::Clamp(_MinX, 0, GridSize.X);
	int ClampedMaxX = FMath::Clamp(_MaxX, 0, GridSize.X);
	int ClampedMinY = FMath::Clamp(_MinY, 0, GridSize.Y);
	int ClampedMaxY = FMath::Clamp(_MaxY, 0, GridSize.Y);

	int RandX = 0;
	int RandY = 0;
	if (_bFreeSlot)
	{
		TArray<FIntVector2D> TempFreeSlots = FreeSlots;
		while (TempFreeSlots.Num() > 0)
		{
			// Get random free slot
			int RandIndex = FMath::RandRange(0, TempFreeSlots.Num() - 1);
			FIntVector2D NewRandCoord = TempFreeSlots[RandIndex];

			// Check within range
			// TODO: In range function?
			if (NewRandCoord.X >= ClampedMinX && NewRandCoord.X <= ClampedMaxX &&
				NewRandCoord.Y >= ClampedMinY && NewRandCoord.Y <= ClampedMaxY)
			{
				// If valid, store and break out of while loop
				RandX = NewRandCoord.X;
				RandY = NewRandCoord.Y;
				break;
			}

			// Random coord not in range, remove from list and try again
			TempFreeSlots.RemoveAt(RandIndex);
		}

		// TODO: Failsafe resolve
		if (TempFreeSlots.Num() <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to find free slot in grid"));
		}
	}
	else
	{
		RandX = FMath::RandRange(ClampedMinX, ClampedMaxX);
		RandY = FMath::RandRange(ClampedMinY, ClampedMaxY);
	}

	return FIntVector2D(RandX, RandY);
}

bool UMM_GridObject::FindFreeSlotInDirection(FIntVector2D& _CurrentPosition, FIntVector2D _Direction)
{
	// Test against next coords
	FIntVector2D TestCoords = _CurrentPosition;
	TestCoords += _Direction;

	// If no slot free, use last valid position
	// Return false as there was no free spot
	if (!FreeSlots.Contains(TestCoords))
		return false;

	// Next slot is available, set to new coords
	_CurrentPosition = TestCoords;

	return true;
}

bool UMM_GridObject::FindFreeSlotBelow(FIntVector2D& _CurrentPosition)
{
	bool FoundFreeSlot = false;

	// If not currently on lowest slot
	while (_CurrentPosition.Y > 0)
	{
		// Check if free slot below
		if (FindFreeSlotInDirection(_CurrentPosition, FIntVector2D(0, -1)))
			FoundFreeSlot = true;
		// No free slot below, exit loop
		else
			break;
	}

	return FoundFreeSlot;
}

bool UMM_GridObject::FindFreeSlotAhead(FIntVector2D& _CurrentPosition, int _Direction)
{
	return FindFreeSlotInDirection(_CurrentPosition, FIntVector2D(_Direction, 0));
}

TArray<FIntVector2D> UMM_GridObject::GetValidPath(FIntVector2D _StartingPosition, int _iHorizontalDirection /*= 1*/)
{
	// Setup initial variables
	TArray<FIntVector2D> Path = { _StartingPosition };
	FIntVector2D LastPosition = _StartingPosition;


	// Loop while valid move
	bool HasMove = true;
	while (HasMove)
	{
		// Set initial valid move to false
		HasMove = false;
		// Start at last position
		FIntVector2D NewPosition = LastPosition;
		// If valid move either ahead or down, has valid move can be set to true
		if (FindFreeSlotBelow(NewPosition))
		{
			Path.Add(NewPosition);
			HasMove = true;
		}
		if (FindFreeSlotAhead(NewPosition, _iHorizontalDirection))
			HasMove = true;

		// If a valid move was found, set last position and store in path
		// Otherwise will exit the while loop, having reached the final position
		if (HasMove)
		{
			LastPosition = NewPosition;
			Path.Add(NewPosition);


		}
	}
	return Path;
}