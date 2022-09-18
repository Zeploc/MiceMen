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
	if (_Coord.X >= GridSize.X || _Coord.Y >= GridSize.Y)
	{
		UE_LOG(MiceMenEventLog, Warning, TEXT("UMM_GridObject::CheckValidCoord | %s not in range of %s"), *_Coord.ToString(), *GridSize.ToString());
		return false;
	}

	// Check grid size is correct 
	if (Grid.Num() != GridSize.X * GridSize.Y)
	{
		UE_LOG(MiceMenEventLog, Warning, TEXT("UMM_GridObject::CheckValidCoord | Grid size incorrect %i is not equal to %i"), Grid.Num(), GridSize.X * GridSize.Y);
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

	// Update grid element if valid/not empty and update FreeSlots map
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
		FreeSlots.Add(_Coord);

		UE_LOG(MiceMenEventLog, Display, TEXT("UMM_GridObject::SetGridElement | Free slot added at %s"), *_Coord.ToString());
	}

	return true;
}

// ################################ Grid Helpers ################################

AMM_GridElement* UMM_GridObject::MoveColumnElements(int _Column, EDirection _Direction)
{
	FIntVector2D LastColumnCoord = { _Column, 0 };
	if (_Direction == EDirection::E_UP)
	{
		LastColumnCoord = { _Column, GridSize.Y - 1 };
	}

	// Find Last element and remove from grid
	AMM_GridElement* LastElement = GetGridElement(LastColumnCoord);
	SetGridElement(LastColumnCoord, nullptr);

	// TODO: Change from iteration to displacing array??
	// At least minimum store in new array, then apply it and update elements

	// Slot the last element into the first slot in the loop
	AMM_GridElement* NextElement = LastElement;

	for (int i = 0; i < GridSize.Y; i++)
	{
		// y based on direction
		// if direction is negative/down, will start at the top and go down
		// Otherwise will start at 0 and go up
		int y = i;
		if (_Direction == EDirection::E_DOWN)
		{
			y = GridSize.Y - 1 - i;
		}

		// Store current element
		FIntVector2D CurrentSlot = { _Column, y };
		AMM_GridElement* CurrentElement = GetGridElement(CurrentSlot);

		// Set current slot to next element stored previously
		SetGridElement(CurrentSlot, NextElement);

#if !UE_BUILD_SHIPPING
		OutputColumnDisplace(NextElement, CurrentElement, CurrentSlot);
#endif

		// Save current element for next
		NextElement = CurrentElement;
	}

	return LastElement;
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
	int ClampedMinX = FMath::Clamp(_MinX, 0, GridSize.X);
	int ClampedMaxX = FMath::Clamp(_MaxX, 0, GridSize.X);
	int ClampedMinY = FMath::Clamp(_MinY, 0, GridSize.Y);
	int ClampedMaxY = FMath::Clamp(_MaxY, 0, GridSize.Y);

	int RandX = -1;
	int RandY = -1;

	// If looking for a free slot
	if (_bFreeSlot)
	{
		// Try each free slot randomly
		TArray<FIntVector2D> TempFreeSlots = FreeSlots;
		while (TempFreeSlots.Num() > 0)
		{
			// Get random free slot
			int RandIndex = FMath::RandRange(0, TempFreeSlots.Num() - 1);
			FIntVector2D NewRandCoord = TempFreeSlots[RandIndex];

			// Check within range
			if (IsCoordInRange(NewRandCoord, ClampedMinX, ClampedMaxX, ClampedMinY, ClampedMaxY))
			{
				// If valid, store and break out of while loop
				RandX = NewRandCoord.X;
				RandY = NewRandCoord.Y;
				break;
			}

			// Random coordinates not in range, remove from list and try again
			TempFreeSlots.RemoveAt(RandIndex);
		}

		// TODO: Failsafe resolve
		if (TempFreeSlots.Num() <= 0)
		{
			UE_LOG(MiceMenEventLog, Error, TEXT("Failed to find free slot in grid"));
		}
	}
	else
	{
		// Slot doesn't need to be free, so simple random range for X and Y
		RandX = FMath::RandRange(ClampedMinX, ClampedMaxX);
		RandY = FMath::RandRange(ClampedMinY, ClampedMaxY);
	}

	return FIntVector2D(RandX, RandY);
}

bool UMM_GridObject::IsCoordInRange(const FIntVector2D& _Coord, int _MinX, int _MaxX, int _MinY, int _MaxY) const
{
	// Check within X range
	bool bWithinX = _Coord.X >= _MinX && _Coord.X <= _MaxX;
	// Check within Y Range
	bool bWithinY = _Coord.Y >= _MinY && _Coord.Y <= _MaxY;

	return (bWithinX && bWithinY);
}

bool UMM_GridObject::FindFreeSlotInDirection(FIntVector2D& _CurrentPosition, FIntVector2D _Direction) const
{
	// Test against next coordinates
	FIntVector2D TestCoords = _CurrentPosition;
	TestCoords += _Direction;

	// If no slot free, CurrentPosition will be unset using the last valid position
	// Return false as there was no free spot
	if (!FreeSlots.Contains(TestCoords))
	{
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
		// Check if free slot below
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
	int HorizontalDirection = _Direction == EDirection::E_RIGHT ? 1 : -1;
	return FindFreeSlotInDirection(_CurrentPosition, FIntVector2D(HorizontalDirection, 0));
}

void UMM_GridObject::OutputColumnDisplace(AMM_GridElement* NextElement, AMM_GridElement* CurrentElement, FIntVector2D& CurrentSlot)
{
	FString NextElementDisplay = "none";
	if (NextElement)
	{
		NextElementDisplay = NextElement->GetName();
	}

	FString CurrentElementDisplay = "none";
	if (CurrentElement)
	{
		CurrentElementDisplay = CurrentElement->GetName();
	}

	UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::AdjustColumn | Setting element %s at %s which was previously %s"), *NextElementDisplay, *CurrentSlot.ToString(), *CurrentElementDisplay);
}