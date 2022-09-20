// Copyright Alex Coultas, Mice Men Example Project


#include "Grid/MM_GridObject.h"

#include "Grid/MM_GridElement.h"
#include "MiceMen.h"

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
		{
			GridElement->CleanUp();
			GridElement->Destroy();
		}
	}
	Grid.Empty();

	FreeSlots.Empty();
}

// ################################ Grid Management ################################

AMM_GridElement* UMM_GridObject::GetGridElement(const FIntVector2D& _Coord) const
{
	if (!IsValidCoord(_Coord))
	{
		UE_LOG(MiceMenEventLog, Warning, TEXT("UMM_GridObject::GetGridElement | %s not valid coordinate"), *_Coord.ToString());
		return nullptr;
	}

	return Grid[CoordToIndex(_Coord.X, _Coord.Y)];
}

bool UMM_GridObject::IsValidCoord(const FIntVector2D& _Coord) const
{
	// Not in grid range
	if (!IsCoordInRange(_Coord, 0, GridSize.X - 1, 0, GridSize.Y - 1))
	{
		UE_LOG(MiceMenEventLog, Warning, TEXT("UMM_GridObject::IsValidCoord | %s not in range of %s"), *_Coord.ToString(), *GridSize.ToString());
		return false;
	}

	// Check grid size is correct
	const int ExpectedGridSize = GridSize.X * GridSize.Y;
	if (Grid.Num() != ExpectedGridSize)
	{
		UE_LOG(MiceMenEventLog, Warning, TEXT("UMM_GridObject::IsValidCoord | Grid size incorrect %i is not equal to %i"), Grid.Num(), ExpectedGridSize);
		return false;
	}

	return true;
}

int UMM_GridObject::CoordToIndex(int _X, int _Y) const
{
	return _X * GridSize.Y + _Y;
}

bool UMM_GridObject::SetGridElement(const FIntVector2D& _Coord, AMM_GridElement* _GridElement)
{
#if !UE_BUILD_SHIPPING
	FString ElementDisplay = "none";
	if (_GridElement)
	{
		ElementDisplay = _GridElement->GetName();
	}
	UE_LOG(MiceMenEventLog, Display, TEXT("UMM_GridObject::SetGridElement | %s to %s"), *_Coord.ToString(), *ElementDisplay);
#endif

	if (!IsValidCoord(_Coord))
	{
		UE_LOG(MiceMenEventLog, Warning, TEXT("UMM_GridObject::SetGridElement | %s not valid coordinate"), *_Coord.ToString());
		return false;
	}

	Grid[CoordToIndex(_Coord.X, _Coord.Y)] = _GridElement;

	// Update grid element if valid/not empty
	if (_GridElement)
	{
		// Only update if a change in coordinates occurred
		if (_GridElement->GetCoordinates() != _Coord)
		{
			_GridElement->UpdateGridPosition(_Coord);
		}

		// Update Free slots to no longer have this element
		FreeSlots.Remove(_Coord);

		UE_LOG(MiceMenEventLog, Display, TEXT("UMM_GridObject::SetGridElement | Free slot removed at %s"), *_Coord.ToString());
	}
	else
	{
		// No element (nullptr), add these coordinates to free slots
		// Note: Add unique is more expensive (but safe), could be improved
		FreeSlots.AddUnique(_Coord);

		UE_LOG(MiceMenEventLog, Display, TEXT("UMM_GridObject::SetGridElement | Free slot added at %s"), *_Coord.ToString());
	}

	return true;
}

// ################################ Grid Helpers ################################

AMM_GridElement* UMM_GridObject::MoveColumnElements(int _Column, EDirection _Direction)
{
	// TODO: Add tests

	// Get initial values
	const int StartingGridIndex = CoordToIndex(_Column, 0);	
	AMM_GridElement* WrappingElement = nullptr;

	const int BottomBlockIndex = StartingGridIndex;
	const int TopBlockIndex = StartingGridIndex + GridSize.Y - 1;

	// Downwards means bottom element wrapping
	if (_Direction == EDirection::E_DOWN)
	{
		WrappingElement = Grid[BottomBlockIndex];
		// Remove bottom element
		Grid.RemoveAt(BottomBlockIndex);
		// Insert element at the top
		Grid.Insert(WrappingElement, TopBlockIndex);
	}
	// Upwards means the top element wrapping
	else if (_Direction == EDirection::E_UP)
	{
		WrappingElement = Grid[TopBlockIndex];
		// Remove top element
		Grid.RemoveAt(TopBlockIndex);
		// Insert element at the top
		Grid.Insert(WrappingElement, BottomBlockIndex);
	}

	// Update each element with its new position and store new free slots
	for (int y = 0; y < GridSize.Y; y++)
	{
		const FIntVector2D ElementCoord = { _Column, y };
		AMM_GridElement* CurrentElement = GetGridElement(ElementCoord);

		// Assign element to new coordinate
		const bool bSetElementSuccess = SetGridElement(ElementCoord, CurrentElement);

		if (!bSetElementSuccess)
		{
			UE_LOG(MiceMenEventLog, Warning, TEXT("UMM_GridObject::MoveColumnElements | Failed to set element at coord %s"), *ElementCoord.ToString());
		}
	}

	return WrappingElement;
}

FIntVector2D UMM_GridObject::GetRandomGridCoord(bool _bFreeSlot /*= true*/) const
{
	return GetRandomGridCoordInRange(0, GridSize.X, 0, GridSize.Y, _bFreeSlot);
}

FIntVector2D UMM_GridObject::GetRandomGridCoordInColumnRange(int _MinX, int _MaxX, bool _bFreeSlot /*= true*/) const
{
	return GetRandomGridCoordInRange(_MinX, _MaxX, 0, GridSize.Y, _bFreeSlot);
}

FIntVector2D UMM_GridObject::GetRandomGridCoordInRange(int _MinX, int _MaxX, int _MinY, int _MaxY, bool _bFreeSlot /*= true*/) const
{
	// Clamp ranges
	const int ClampedMinX = FMath::Clamp(_MinX, 0, GridSize.X);
	const int ClampedMaxX = FMath::Clamp(_MaxX, 0, GridSize.X);
	const int ClampedMinY = FMath::Clamp(_MinY, 0, GridSize.Y);
	const int ClampedMaxY = FMath::Clamp(_MaxY, 0, GridSize.Y);

	int RandX = -1;
	int RandY = -1;

	// If looking for a free slot
	if (_bFreeSlot)
	{
		// Try each free slot randomly
		TArray<FIntVector2D> AvailableFreeSlots = FreeSlots;
		while (AvailableFreeSlots.Num() > 0)
		{
			// Get random free slot
			const int RandIndex = FMath::RandRange(0, AvailableFreeSlots.Num() - 1);
			FIntVector2D NewRandCoord = AvailableFreeSlots[RandIndex];

			// Check within range
			if (IsCoordInRange(NewRandCoord, ClampedMinX, ClampedMaxX, ClampedMinY, ClampedMaxY))
			{
				// If valid, store and break out of while loop
				RandX = NewRandCoord.X;
				RandY = NewRandCoord.Y;
				break;
			}

			// Remove from list and try again as random coordinates not in range
			AvailableFreeSlots.RemoveAt(RandIndex);
		}

		// TODO: Failsafe resolve
		if (AvailableFreeSlots.Num() <= 0)
		{
			UE_LOG(MiceMenEventLog, Error, TEXT("Failed to find free slot in grid"));
		}
	}
	else
	{
		// Slot doesn't need to be free, so standard random range for X and Y
		RandX = FMath::RandRange(ClampedMinX, ClampedMaxX);
		RandY = FMath::RandRange(ClampedMinY, ClampedMaxY);
	}

	return FIntVector2D(RandX, RandY);
}

bool UMM_GridObject::IsCoordInRange(const FIntVector2D& _Coord, int _MinX, int _MaxX, int _MinY, int _MaxY)
{
	// Check within X range
	const bool bWithinX = _Coord.X >= _MinX && _Coord.X <= _MaxX;
	// Check within Y Range
	const bool bWithinY = _Coord.Y >= _MinY && _Coord.Y <= _MaxY;

	return (bWithinX && bWithinY);
}

bool UMM_GridObject::FindFreeSlotInDirection(FIntVector2D& _CurrentPosition, FIntVector2D _Direction) const
{
	// Test against next coordinates
	FIntVector2D TestCoords = _CurrentPosition;
	TestCoords += _Direction;

	// Checks if there is no free slot
	if (!FreeSlots.Contains(TestCoords))
	{
		// CurrentPosition will be unset using the last valid position
		return false;
	}

	// Next slot is available, set to new coordinates
	_CurrentPosition = TestCoords;

	return true;
}

bool UMM_GridObject::FindFreeSlotBelow(FIntVector2D& _CurrentPosition) const
{
	bool bFoundFreeSlot = false;

	// If not currently on lowest slot
	while (_CurrentPosition.Y > 0)
	{
		// Check if free slot below, will update _CurrentPosition 
		if (FindFreeSlotInDirection(_CurrentPosition, FIntVector2D(0, -1)))
		{
			bFoundFreeSlot = true;
		}
		// No free slot below, exit loop
		else
		{
			break;
		}
	}

	return bFoundFreeSlot;
}

bool UMM_GridObject::FindFreeSlotAhead(FIntVector2D& _CurrentPosition, EDirection _Direction) const
{
	const int HorizontalDirection = _Direction == EDirection::E_RIGHT ? 1 : -1;
	return FindFreeSlotInDirection(_CurrentPosition, FIntVector2D(HorizontalDirection, 0));
}
