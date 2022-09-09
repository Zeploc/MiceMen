// Copyright Alex Coultas, Mice Men Example Project


#include "Grid/MM_GridManager.h"

#include "Kismet/GameplayStatics.h"

#include "MM_GridElement.h"
#include "MM_GridBlock.h"
#include "Gameplay/MM_Mouse.h"
#include "Gameplay/MM_ColumnControl.h"

// Sets default values
AMM_GridManager::AMM_GridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GridSize = FIntVector2D(19, 13);
	GridBlockClass = AMM_GridBlock::StaticClass();
	MouseClass = AMM_Mouse::StaticClass();
	ColumnControlClass = AMM_ColumnControl::StaticClass();
}

// Called when the game starts or when spawned
void AMM_GridManager::BeginPlay()
{
	Super::BeginPlay();

	RebuildGrid();

}

void AMM_GridManager::RebuildGrid()
{
	GridCleanUp();

	SetupGrid();
	PopulateMice();
}
void AMM_GridManager::GridCleanUp()
{
	for (AMM_GridElement* GridInfo : Grid)
	{
		GridInfo->CleanUp();
	}
	Grid.Empty();
	FreeSlots.Empty();
	FreeSlotKeys.Empty();
}

void AMM_GridManager::SetupGrid()
{
	if (GridSize.X < 2 || GridSize.Y < 2)
	{
		UE_LOG(LogTemp, Error, TEXT("GRID TOO SMALL, CANNOT SETUP"));
		return;
	}

	// Remainder of divide by 2, either 0 or 1
	GapSize = GridSize.X % 2;
	// Team size is the amount without the gap, halved
	TeamSize = (GridSize.X - GapSize) / 2;

	Grid.SetNumZeroed(GridSize.X * GridSize.Y);

	// For each column
	for (int x = 0; x < GridSize.X; x++)
	{
		// Check class is valid
		if (!ColumnControlClass)
		{
			ColumnControlClass = AMM_ColumnControl::StaticClass();
		}
		FTransform ColumnTransform = GetWorldTransformFromCoord(FIntVector2D(x, 0));
		AMM_ColumnControl* NewColumnControl = GetWorld()->SpawnActor<AMM_ColumnControl>(ColumnControlClass, ColumnTransform);
		NewColumnControl->SetupColumn(x, this);
		ColumnControls.Add(x, NewColumnControl);
		

		// For each row, add to column array
		for (int y = 0; y < GridSize.Y; y++)
		{
			// Initial variables
			FIntVector2D NewPosition = FIntVector2D(x, y);
			FTransform GridElementTransform = GetWorldTransformFromCoord(NewPosition);

			// Prepare center pieces
			// TODO: Change from hardcode
			// TODO IMPROVE LOGIC
			bool IsPresetCenterBlock = x >= 8 && x <= 10;
			bool isPresetBlockFilled = false;
			if (IsPresetCenterBlock)
			{
				// Center column
				if (x == 9)
					// If its not every third block
					isPresetBlockFilled = y % 3 != 0;
				else if ((y + 3) % 3 == 0)
					isPresetBlockFilled = true;
			}

			// Initial testing random bool for placement
			if ((FMath::RandBool() && !IsPresetCenterBlock) || isPresetBlockFilled)
			{
				// Create new Grid block object and add to column array
				AMM_GridBlock* NewGridBlock = GetWorld()->SpawnActor<AMM_GridBlock>(GridBlockClass, GridElementTransform);
				if (!NewGridBlock)
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to create Grid Block!"));
					continue;
				}
				NewGridBlock->SetupGridInfo(NewPosition);
				NewGridBlock->AttachToActor(NewColumnControl, FAttachmentTransformRules::KeepWorldTransform);
				SetGridElement(FIntVector2D(x, y), NewGridBlock);

				// TODO: NOT NEEDED? Safety?
				// Update FreeSlots map
				//FreeSlots.Remove(NewPosition);
			}
			else
			{
				// Update FreeSlots map
				FreeSlots.Add(NewPosition, CoordToIndex(x, y));
			}
		}
	}

	// Update FreeSlots array
	FreeSlots.GenerateKeyArray(FreeSlotKeys);
}

void AMM_GridManager::PopulateMice()
{
	FIntVector2D TeamRanges[] = {
		FIntVector2D(0, TeamSize - 1),
		FIntVector2D(TeamSize + GapSize, GridSize.X - 1)
	};

	for (int iTeam = 0; iTeam < 2; iTeam++)
	{
		for (int iMouse = 0; iMouse < InitialMiceCount; iMouse++)
		{
			FIntVector2D NewRandomMousePosition = GetRandomGridCoordInColumnRange(TeamRanges[iTeam].X, TeamRanges[iTeam].Y);

			// Move to final position
			TArray<FIntVector2D> ValidPath = GetValidPath(NewRandomMousePosition, iTeam == 0 ? 1 : -1);
			NewRandomMousePosition = ValidPath.Last();

			// Get coords in world
			FTransform GridElementTransform = GetWorldTransformFromCoord(NewRandomMousePosition);

			// Setup new Mouse
			AMM_Mouse* NewMouse = GetWorld()->SpawnActorDeferred<AMM_Mouse>(MouseClass, GridElementTransform);
			NewMouse->SetupGridInfo(NewRandomMousePosition);
			NewMouse->SetupMouse(iTeam);
			UGameplayStatics::FinishSpawningActor(NewMouse, GridElementTransform);
			NewMouse->AttachToActor(ColumnControls[NewRandomMousePosition.X], FAttachmentTransformRules::KeepWorldTransform);
			Mice.Add(NewMouse);

			// TODO: Can either use math to calc from coord to index, or use free slots map by hash
			SetGridElement(NewRandomMousePosition, NewMouse);

			// Update FreeSlots map
			FreeSlots.Remove(NewRandomMousePosition);

			// Update FreeSlots array
			FreeSlots.GenerateKeyArray(FreeSlotKeys);
		}
	}

}


bool AMM_GridManager::FindFreeSlotInDirection(FIntVector2D& _CurrentPosition, FIntVector2D _Direction)
{
	// Test against next coords
	FIntVector2D TestCoords = _CurrentPosition;
	TestCoords += _Direction;

	// If no slot free, use last valid position
	// Return false as there was no free spot
	if (!FreeSlotKeys.Contains(TestCoords))
		return false;

	// Next slot is available, set to new coords
	_CurrentPosition = TestCoords;

	return true;
}

bool AMM_GridManager::FindFreeSlotBelow(FIntVector2D& _CurrentPosition)
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

bool AMM_GridManager::FindFreeSlotAhead(FIntVector2D& _CurrentPosition, int _Direction)
{
	return FindFreeSlotInDirection(_CurrentPosition, FIntVector2D(_Direction, 0));
}

TArray<FIntVector2D> AMM_GridManager::GetValidPath(FIntVector2D _StartingPosition, int _iHorizontalDirection /*= 1*/)
{
	// Setup initial variables
	TArray<FIntVector2D> Path = { _StartingPosition};
	FIntVector2D LastPosition = _StartingPosition;

	auto colour = FLinearColor::MakeRandomColor();

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
			UKismetSystemLibrary::DrawDebugBox(GetWorld(), GetWorldTransformFromCoord(NewPosition).GetLocation() + FVector(0, 0, 50), FVector(40 * ((float)Path.Num() / (float)10) + 5), colour, FRotator::ZeroRotator, 5, 3);
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

			UKismetSystemLibrary::DrawDebugBox(GetWorld(), GetWorldTransformFromCoord(NewPosition).GetLocation() + FVector(0, 0, 50), FVector(40 * ((float)Path.Num() / (float)10) + 5), colour, FRotator::ZeroRotator, 5, 3);

		}
	}
	return Path;
}

TArray<FVector> AMM_GridManager::PathFromCoordToWorld(TArray<FIntVector2D> _CoordPath)
{
	TArray<FVector> NewWorldPath;
	for (FIntVector2D _Coord : _CoordPath)
	{		
		NewWorldPath.Add(GetWorldTransformFromCoord(_Coord).GetLocation());
	}
	return NewWorldPath;
}

AMM_GridElement* AMM_GridManager::GetGridElement(FIntVector2D _Coord)
{
	// Not in grid range
	if (_Coord.X >= GridSize.X || _Coord.Y >= GridSize.Y)
		return nullptr;

	// Check grid size is correct 
	if (Grid.Num() != GridSize.X * GridSize.Y)
		return nullptr;

	return Grid[CoordToIndex(_Coord.X, _Coord.Y)];
}

int AMM_GridManager::CoordToIndex(int _X, int _Y)
{
	return _X * GridSize.Y + _Y;
}

bool AMM_GridManager::SetGridElement(FIntVector2D _Coord, class AMM_GridElement* _GridElement)
{
	// Not in grid range
	if (_Coord.X >= GridSize.X || _Coord.Y >= GridSize.Y)
		return false;

	// Check grid size is correct 
	if (Grid.Num() != GridSize.X * GridSize.Y)
		return false;

	Grid[CoordToIndex(_Coord.X, _Coord.Y)] = _GridElement;

	// Update grid element if valid/not empty
	// Update FreeSlots map
	if (_GridElement)
	{
		_GridElement->UpdateGridPosition(_Coord);
		FreeSlots.Remove(_Coord);
	}
	else
		FreeSlots.Add(_Coord);

	return true;
}

FTransform AMM_GridManager::GetWorldTransformFromCoord(FIntVector2D _Coords)
{
	FTransform GridElementTransform = GetActorTransform();
	FVector NewRelativeLocation = FVector::Zero();

	// X is forward, so horizontal axis is Y, and vertical axis is Z
	NewRelativeLocation.Y -= (GridSize.X / 2) * GridElementWidth;
	NewRelativeLocation.Y += _Coords.X * GridElementWidth;
	NewRelativeLocation.Z += _Coords.Y * GridElementHeight;

	// NEEDS TO BE RELATIVE LOCATION TO TRANSFORM
	GridElementTransform.SetLocation(NewRelativeLocation);

	return GridElementTransform;
}

FIntVector2D AMM_GridManager::GetRandomGridCoord(bool _bFreeSlot /*= true*/)
{
	return GetRandomGridCoordInRange(0, GridSize.X, 0, GridSize.Y, _bFreeSlot);
}

FIntVector2D AMM_GridManager::GetRandomGridCoordInColumnRange(int _MinX, int _MaxX, bool _bFreeSlot /*= true*/)
{
	return GetRandomGridCoordInRange(_MinX, _MaxX, 0, GridSize.Y, _bFreeSlot);
}

FIntVector2D AMM_GridManager::GetRandomGridCoordInRange(int _MinX, int _MaxX, int _MinY, int _MaxY, bool _bFreeSlot /*= true*/)
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
		TArray<FIntVector2D> TempFreeSlots = FreeSlotKeys;
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

void AMM_GridManager::ProcessMice()
{
	for (AMM_Mouse* Mouse : Mice)
	{
		// Get Path
		TArray<FIntVector2D> ValidPath = GetValidPath(Mouse->GetCoordinates(), Mouse->GetTeam() == 0 ? 1 : -1);

		// Make movement
		Mouse->MoveAlongPath(PathFromCoordToWorld(ValidPath));		
		SetGridElement(Mouse->GetCoordinates(), nullptr);
		SetGridElement(ValidPath.Last(), Mouse);


		// Update FreeSlots array
		FreeSlots.GenerateKeyArray(FreeSlotKeys);
	}
}

// Called every frame
void AMM_GridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	return;

	// DEBUG FOR GRID VALUES
	// For each column
	for (int x = 0; x < GridSize.X; x++)
	{

		// For each row, add to column array
		for (int y = 0; y < GridSize.Y; y++)
		{
			AMM_GridElement* gridElement = GetGridElement({ x, y });
			auto color = FLinearColor::White;
			if (gridElement)
			{
				color = FLinearColor::Yellow;
				if (gridElement->IsA(AMM_Mouse::StaticClass()))
					color = FLinearColor::Green;
			}
			UKismetSystemLibrary::DrawDebugSphere(GetWorld(), GetWorldTransformFromCoord({ x, y }).GetLocation() + FVector(-100, 0, 50), 25.0f, 6, color, 0.5, 3);
		}
	}

}

void AMM_GridManager::AdjustColumn(int _Column, int _Direction)
{
	FIntVector2D LastColumnCoord = { _Column, 0 };
	if (_Direction > 0)
		LastColumnCoord = { _Column, GridSize.Y - 1 };

	// Find Last element and remove from grid
	AMM_GridElement* LastElement = GetGridElement(LastColumnCoord);
	SetGridElement(LastColumnCoord, nullptr);

	// Slot the last element into the first slot in the loop
	AMM_GridElement* NextElement = LastElement;

	for (int i = 0; i < GridSize.Y; i++)
	{
		// y based on direction
		// if direction is negative/down, will start at the top and go down
		// Otherwise will start at 0 and go up
		int y = i;
		if (_Direction < 0)
			y = GridSize.Y - 1 - i;

		// Store current element
		FIntVector2D CurrentSlot = { _Column, y };
		AMM_GridElement* CurrentElement = GetGridElement(CurrentSlot);

		// Set current slot to next element stored previously
		SetGridElement(CurrentSlot, NextElement);

		// Save current element for next
		NextElement = CurrentElement;

		// First element set to last
		if (i == 0)
		{
			// TODO: Animate/visual
			// Update world position
			if (LastElement)
				LastElement->SetActorLocation(GetWorldTransformFromCoord(CurrentSlot).GetLocation());
		}

	}

	// Update FreeSlots array
	FreeSlots.GenerateKeyArray(FreeSlotKeys);

	ProcessMice();
}

