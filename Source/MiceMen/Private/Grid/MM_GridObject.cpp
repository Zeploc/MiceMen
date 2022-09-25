// Copyright Alex Coultas, Mice Men Example Project

#include "Grid/MM_GridObject.h"

#include "Grid/MM_GridElement.h"
#include "MiceMen.h"

void UMM_GridObject::SetupGrid(const FIntVector2D& _GridSize)
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

AMM_GridElement* UMM_GridObject::GetGridElement(const FIntVector2D& Coord) const
{
	if (!IsValidCoord(Coord))
	{
		UE_LOG(MiceMenEventLog, Warning, TEXT("UMM_GridObject::GetGridElement | %s not valid coordinate"), *Coord.ToString());
		return nullptr;
	}

	return Grid[CoordToIndex(Coord.X, Coord.Y)];
}

bool UMM_GridObject::IsValidCoord(const FIntVector2D& Coord) const
{
	// Not in grid range
	if (!IsCoordInRange(Coord, 0, GridSize.X - 1, 0, GridSize.Y - 1))
	{
		UE_LOG(MiceMenEventLog, Warning, TEXT("UMM_GridObject::IsValidCoord | %s not in range of %s"), *Coord.ToString(), *GridSize.ToString());
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

int UMM_GridObject::CoordToIndex(int X, int Y) const
{
	return X * GridSize.Y + Y;
}

bool UMM_GridObject::SetGridElement(const FIntVector2D& Coord, AMM_GridElement* GridElement)
{
	if (!IsValidCoord(Coord))
	{
		UE_LOG(MiceMenEventLog, Warning, TEXT("UMM_GridObject::SetGridElement | %s not valid coordinate"), *Coord.ToString());
		return false;
	}

#if !UE_BUILD_SHIPPING
	FString ElementDisplay = "none";
	if (GridElement)
	{
		ElementDisplay = GridElement->GetName();
	}
	UE_LOG(MiceMenEventLog, Display, TEXT("UMM_GridObject::SetGridElement | %s to %s"), *Coord.ToString(), *ElementDisplay);
#endif

	Grid[CoordToIndex(Coord.X, Coord.Y)] = GridElement;

	// Update grid element if valid/not empty
	if (GridElement)
	{
		// Only update if a change in coordinates occurred
		if (GridElement->GetCoordinates() != Coord)
		{
			GridElement->UpdateGridPosition(Coord);
		}

		// Update Free slots to no longer have this element
		FreeSlots.Remove(Coord);

		UE_LOG(MiceMenEventLog, Display, TEXT("UMM_GridObject::SetGridElement | Free slot removed at %s"), *Coord.ToString());
	}
	else
	{
		// No element (nullptr), add these coordinates to free slots
		// Note: Add unique is more expensive (but safe), could be improved
		FreeSlots.AddUnique(Coord);

		UE_LOG(MiceMenEventLog, Display, TEXT("UMM_GridObject::SetGridElement | Free slot added at %s"), *Coord.ToString());
	}

	return true;
}

bool UMM_GridObject::MoveGridElement(const FIntVector2D& NewCoord, AMM_GridElement* GridElement)
{
	if (!GridElement)
	{
		return false;
	}

	if (!IsValidCoord(NewCoord))
	{
		UE_LOG(MiceMenEventLog, Warning, TEXT("UMM_GridObject::MoveGridElement | %s not valid coordinate"), *NewCoord.ToString());
		return false;
	}

	// Get original location to move from
	const FIntVector2D OriginalCoordinate = GridElement->GetCoordinates();

	// Check the new coordinates aren't the same as the current
	if (OriginalCoordinate == NewCoord)
	{
		return false;
	}

	UE_LOG(MiceMenEventLog, Display, TEXT("UMM_GridObject::MoveGridElement | Moving %s to %s"), *GridElement->GetName(), *NewCoord.ToString());

	Grid[CoordToIndex(OriginalCoordinate.X, OriginalCoordinate.Y)] = nullptr;
	Grid[CoordToIndex(NewCoord.X, NewCoord.Y)] = GridElement;

	GridElement->UpdateGridPosition(NewCoord);

	// Update FreeSlots to have the old position as free
	// Note: Add unique is more expensive (but safe), could be improved
	FreeSlots.AddUnique(OriginalCoordinate);

	// Remove the new space as free as there is now an element there
	FreeSlots.Remove(NewCoord);

	return true;
}

// ################################ Grid Helpers ################################

AMM_GridElement* UMM_GridObject::MoveColumnElements(int Column, EDirection Direction)
{
	// Get initial values
	const int StartingGridIndex = CoordToIndex(Column, 0);
	AMM_GridElement* WrappingElement = nullptr;

	const int BottomBlockIndex = StartingGridIndex;
	const int TopBlockIndex = StartingGridIndex + GridSize.Y - 1;

	// Downwards means bottom element wrapping
	if (Direction == EDirection::E_DOWN)
	{
		WrappingElement = Grid[BottomBlockIndex];
		// Remove bottom element
		Grid.RemoveAt(BottomBlockIndex);
		// Insert element at the top
		Grid.Insert(WrappingElement, TopBlockIndex);
	}
	// Upwards means the top element wrapping
	else if (Direction == EDirection::E_UP)
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
		const FIntVector2D ElementCoord = {Column, y};
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

FIntVector2D UMM_GridObject::GetRandomGridCoord(bool bFreeSlot /*= true*/) const
{
	return GetRandomGridCoordInRange(0, GridSize.X, 0, GridSize.Y, bFreeSlot);
}

FIntVector2D UMM_GridObject::GetRandomGridCoordInColumnRange(int MinX, int MaxX, bool bFreeSlot /*= true*/) const
{
	return GetRandomGridCoordInRange(MinX, MaxX, 0, GridSize.Y, bFreeSlot);
}

FIntVector2D UMM_GridObject::GetRandomGridCoordInRange(int MinX, int MaxX, int MinY, int MaxY, bool bFreeSlot /*= true*/) const
{
	// Clamp ranges
	const int ClampedMinX = FMath::Clamp(MinX, 0, GridSize.X);
	const int ClampedMaxX = FMath::Clamp(MaxX, 0, GridSize.X);
	const int ClampedMinY = FMath::Clamp(MinY, 0, GridSize.Y);
	const int ClampedMaxY = FMath::Clamp(MaxY, 0, GridSize.Y);

	int RandX = -1;
	int RandY = -1;

	// If looking for a free slot
	if (bFreeSlot)
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

		// No valid position was found, indicates problem with grid generation
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

bool UMM_GridObject::IsCoordInRange(const FIntVector2D& Coord, int MinX, int MaxX, int MinY, int MaxY)
{
	// Check within X range
	const bool bWithinX = Coord.X >= MinX && Coord.X <= MaxX;
	// Check within Y Range
	const bool bWithinY = Coord.Y >= MinY && Coord.Y <= MaxY;

	return (bWithinX && bWithinY);
}

bool UMM_GridObject::FindFreeSlotInDirection(FIntVector2D& CurrentPosition, const FIntVector2D& Direction) const
{
	// Test against next coordinates
	FIntVector2D TestCoords = CurrentPosition;
	TestCoords += Direction;

	// Checks if there is no free slot
	if (!FreeSlots.Contains(TestCoords))
	{
		// CurrentPosition will be unset using the last valid position
		return false;
	}

	// Next slot is available, set to new coordinates
	CurrentPosition = TestCoords;

	return true;
}

bool UMM_GridObject::FindFreeSlotBelow(FIntVector2D& CurrentPosition) const
{
	bool bFoundFreeSlot = false;

	// If not currently on lowest slot
	while (CurrentPosition.Y > 0)
	{
		// Check if free slot below, will update CurrentPosition 
		if (FindFreeSlotInDirection(CurrentPosition, FIntVector2D(0, -1)))
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

bool UMM_GridObject::FindFreeSlotAhead(FIntVector2D& CurrentPosition, EDirection Direction) const
{
	const int HorizontalDirection = Direction == EDirection::E_RIGHT ? 1 : -1;
	return FindFreeSlotInDirection(CurrentPosition, FIntVector2D(HorizontalDirection, 0));
}
