// Copyright Alex Coultas, Mice Men Example Project


#include "Grid/MM_GridManager.h"

#include "MM_GridInfo.h"
#include "MM_GridBlock.h"
#include "Gameplay/MM_Mouse.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
AMM_GridManager::AMM_GridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GridSize = FIntVector(19, 13, 0);
	GridBlockClass = AMM_GridBlock::StaticClass();
	MouseClass = AMM_Mouse::StaticClass();
}

// Called when the game starts or when spawned
void AMM_GridManager::BeginPlay()
{
	Super::BeginPlay();
	
	SetupGrid();
	PopulateGridBlocks();
	PopulateMice();

	// TODO: Temp
	//ProcessMice();
}

void AMM_GridManager::SetupGrid()
{
	// For each column
	for (int x = 0; x < GridSize.X; x++)
	{
		// For each row, add to column array
		for (int y = 0; y < GridSize.Y; y++)
		{
			// Create new Grid info object and add to column array
			UMM_GridInfo* NewGridObject = NewObject<UMM_GridInfo>(UMM_GridInfo::StaticClass());
			if (!NewGridObject)
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to create Grid Object!"));
				// Insert empty
				// NOTE: Could instead add once at the end, instead of two points of adding to the array
				Grid.Add(nullptr);
				continue;
			}
			// Setup and store
			FIntVector NewPosition = FIntVector(x, y, 0);
			NewGridObject->SetupGridInfo(NewPosition);
			Grid.Add(NewGridObject);

			// Update FreeSlots map
			FreeSlots.Add(NewPosition, CoordToIndex(x, y));
		}
	}

	// Update FreeSlots array
	FreeSlots.GenerateKeyArray(FreeSlotKeys);
}

void AMM_GridManager::PopulateGridBlocks()
{
	// Map is incorrect size
	if (Grid.Num() != GridSize.X * GridSize.Y)
		return;

	// For each grid slot
	for (int x = 0; x < GridSize.X; x++)
	{
		for (int y = 0; y < GridSize.Y; y++)
		{
			// Initial variables
			FTransform GridElementTransform = GetWorldTransformFromCoord(FIntVector(x, y, 0));

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
				AMM_GridBlock* NewGridBlock = GetWorld()->SpawnActor<AMM_GridBlock>(GridBlockClass, GridElementTransform);
				Grid[CoordToIndex(x, y)]->SetGridBlock(NewGridBlock);

				// Update FreeSlots map
				FreeSlots.Remove(FIntVector(x, y, 0));
			}
		}
	}

	// Update FreeSlots array
	FreeSlots.GenerateKeyArray(FreeSlotKeys);
}

void AMM_GridManager::PopulateMice()
{
	// Remainder of divide by 2, either 0 or 1
	int GapSize = GridSize.X % 2;
	// Team size is the amount without the gap, halved
	int TeamSize = (GridSize.X - GapSize) / 2;

	FIntVector TeamRanges[] = {
		FIntVector(0, TeamSize - 1, 0),
		FIntVector(TeamSize + GapSize, GridSize.X - 1, 0)
	};

	for (int iTeam = 0; iTeam < 2; iTeam++)
	{
		for (int iMouse = 0; iMouse < InitialMiceCount; iMouse++)
		{
			FIntVector NewRandomMousePosition = GetRandomGridCoordInColumnRange(TeamRanges[iTeam].X, TeamRanges[iTeam].Y);

			// Move to final position
			TArray<FIntVector> ValidPath = GetValidPath(NewRandomMousePosition, iTeam == 0 ? 1 : -1);
			NewRandomMousePosition = ValidPath.Last();

			// Get coords in world
			FTransform GridElementTransform = GetWorldTransformFromCoord(NewRandomMousePosition);

			// Setup new Mouse
			AMM_Mouse* NewMouse = GetWorld()->SpawnActorDeferred<AMM_Mouse>(MouseClass, GridElementTransform);
			NewMouse->SetupMouse(iTeam, NewRandomMousePosition);
			UGameplayStatics::FinishSpawningActor(NewMouse, GridElementTransform);
			Mice.Add(NewMouse);

			// TODO: Can either use math to calc from coord to index, or use free slots map by hash
			Grid[CoordToIndex(NewRandomMousePosition.X, NewRandomMousePosition.Y)]->SetMouse(NewMouse);

			// Update FreeSlots map
			FreeSlots.Remove(NewRandomMousePosition);

			// Update FreeSlots array
			FreeSlots.GenerateKeyArray(FreeSlotKeys);
		}
	}

}

bool AMM_GridManager::FindFreeSlotInDirection(FIntVector& _CurrentPosition, FIntVector _Direction)
{
	// Test against next coords
	FIntVector TestCoords = _CurrentPosition;
	TestCoords += _Direction;

	// If no slot free, use last valid position
	// Return false as there was no free spot
	if (!FreeSlotKeys.Contains(TestCoords))
		return false;

	// Next slot is available, set to new coords
	_CurrentPosition = TestCoords;

	return true;
}

bool AMM_GridManager::FindFreeSlotBelow(FIntVector& _CurrentPosition)
{
	bool FoundFreeSlot = false;

	// If not currently on lowest slot
	while (_CurrentPosition.Y > 0)
	{
		// Check if free slot below
		if (FindFreeSlotInDirection(_CurrentPosition, FIntVector(0, -1, 0)))
			FoundFreeSlot = true;
		// No free slot below, exit loop
		else
			break;
	}

	return FoundFreeSlot;
}

bool AMM_GridManager::FindFreeSlotAhead(FIntVector& _CurrentPosition, int _Direction)
{
	return FindFreeSlotInDirection(_CurrentPosition, FIntVector(_Direction, 0, 0));
}

TArray<FIntVector> AMM_GridManager::GetValidPath(FIntVector _StartingPosition, int _iHorizontalDirection /*= 1*/)
{
	// Setup initial variables
	TArray<FIntVector> Path = { _StartingPosition };
	FIntVector LastPosition = _StartingPosition;

	auto colour = FLinearColor::MakeRandomColor();

	// Loop while valid move
	bool HasMove = true;
	while (HasMove)
	{
		// Set initial valid move to false
		HasMove = false;
		// Start at last position
		FIntVector NewPosition = LastPosition;
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

			UKismetSystemLibrary::DrawDebugBox(GetWorld(), GetWorldTransformFromCoord(NewPosition).GetLocation() + FVector(0, 0, 50), FVector(40 * ((float)Path.Num() / (float)10) + 5), colour, FRotator::ZeroRotator, 100000, 3);

		}
	}
	return Path;
}

TArray<FVector> AMM_GridManager::PathFromCoordToWorld(TArray<FIntVector> _CoordPath)
{
	TArray<FVector> NewWorldPath;
	for (FIntVector _Coord : _CoordPath)
	{		
		NewWorldPath.Add(GetWorldTransformFromCoord(_Coord).GetLocation());
	}
	return NewWorldPath;
}

int AMM_GridManager::CoordToIndex(int _X, int _Y)
{
	return _X * GridSize.Y + _Y;
}

// TODO: Change to vec2
FTransform AMM_GridManager::GetWorldTransformFromCoord(FIntVector _Coords)
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

FIntVector AMM_GridManager::GetRandomGridCoord(bool _bFreeSlot /*= true*/)
{
	return GetRandomGridCoordInRange(0, GridSize.X, 0, GridSize.Y, _bFreeSlot);
}

FIntVector AMM_GridManager::GetRandomGridCoordInColumnRange(int _MinX, int _MaxX, bool _bFreeSlot /*= true*/)
{
	return GetRandomGridCoordInRange(_MinX, _MaxX, 0, GridSize.Y, _bFreeSlot);
}

FIntVector AMM_GridManager::GetRandomGridCoordInRange(int _MinX, int _MaxX, int _MinY, int _MaxY, bool _bFreeSlot /*= true*/)
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
		TArray<FIntVector> TempFreeSlots = FreeSlotKeys;
		while (TempFreeSlots.Num() > 0)
		{
			// Get random free slot
			int RandIndex = FMath::RandRange(0, TempFreeSlots.Num() - 1);
			FIntVector NewRandCoord = TempFreeSlots[RandIndex];

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

	return FIntVector(RandX, RandY, 0);
}

void AMM_GridManager::ProcessMice()
{
	for (AMM_Mouse* Mouse : Mice)
	{
		// Get Path
		TArray<FIntVector> ValidPath = GetValidPath(Mouse->Coords, Mouse->GetTeam() == 0 ? 1 : -1);

		// Make movement
		Mouse->MoveAlongPath(PathFromCoordToWorld(ValidPath));		
	}
}

// Called every frame
void AMM_GridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

