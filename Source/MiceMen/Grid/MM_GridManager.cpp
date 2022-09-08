// Copyright Alex Coultas, Mice Men Example Project


#include "Grid/MM_GridManager.h"
#include "MM_GridInfo.h"
#include "MM_GridBlock.h"

// Sets default values
AMM_GridManager::AMM_GridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GridSize = FIntVector(19, 13, 0);
	GridBlockClass = AMM_GridBlock::StaticClass();
}

// Called when the game starts or when spawned
void AMM_GridManager::BeginPlay()
{
	Super::BeginPlay();
	
	SetupGrid();
	PopulateGridBlocks();
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
			NewGridObject->SetupGridInfo(FIntVector(x, y, 0));
			Grid.Add(NewGridObject);

		}
	}
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
			FTransform GridElementTransform = GetWorldTransformFromCoord(x, y);


			// Initial testing random bool for placement
			if (FMath::RandBool())
			{
				AMM_GridBlock* NewGridBlock = GetWorld()->SpawnActor<AMM_GridBlock>(GridBlockClass, GridElementTransform);
				Grid[x * GridSize.Y + y]->SetGridBlock(NewGridBlock);

				// TODO: Update FreeSlots map
			}
		}
	}
}

void AMM_GridManager::PopulateMice()
{
	// Remainder of divide by 2, either 0 or 1
	int GapSize = GridSize.X % 2;
	// Team size is the amount without the gap, halved
	int TeamSize = (GridSize.X - GapSize) / 2;

	FIntVector TeamRanges[] = {
		FIntVector(0, TeamSize, 0),
		FIntVector(TeamSize + GapSize, GridSize.X, 0)
	};

	for (int iTeam = 0; iTeam < 2; iTeam++)
	{
		for (int iMouse = 0; iMouse < InitialMiceCount; iMouse++)
		{
			FIntVector NewRandomMousePosition = GetRandomGridCoordInColumnRange(TeamRanges[iTeam].X, TeamRanges[iTeam].Y);
			// TODO: Add bFreeSlot flag to GetRandomGridCoordInColumnRange(), use FreeSlots map/array
		}
	}

}

int AMM_GridManager::CoordToIndex(int _X, int _Y)
{
	return _X * GridSize.Y + _Y;
}

FTransform AMM_GridManager::GetWorldTransformFromCoord(int _X, int _Y)
{
	FTransform GridElementTransform = GetActorTransform();
	FVector NewRelativeLocation = FVector::Zero();

	// X is forward, so horizontal axis is Y, and vertical axis is Z
	NewRelativeLocation.Y -= (GridSize.X / 2) * GridElementWidth;
	NewRelativeLocation.Y += _X * GridElementWidth;
	NewRelativeLocation.Z += _Y * GridElementHeight;

	// NEEDS TO BE RELATIVE LOCATION TO TRANSFORM
	GridElementTransform.SetLocation(NewRelativeLocation);

	return GridElementTransform;
}

FIntVector AMM_GridManager::GetRandomGridCoord()
{
	GetRandomGridCoordInRange(0, GridSize.X, 0, GridSize.Y);
}

FIntVector AMM_GridManager::GetRandomGridCoordInColumnRange(int _MinX, int _MaxX)
{
	GetRandomGridCoordInRange(_MinX, _MaxX, 0, GridSize.Y);
}

FIntVector AMM_GridManager::GetRandomGridCoordInRange(int _MinX, int _MaxX, int _MinY, int _MaxY)
{
	// Clamp ranges
	int ClampedMinX = FMath::Clamp(_MinX, 0, GridSize.X);
	int ClampedMaxX = FMath::Clamp(_MaxX, 0, GridSize.X);
	int ClampedMinY = FMath::Clamp(_MinY, 0, GridSize.Y);
	int ClampedMaxY = FMath::Clamp(_MaxY, 0, GridSize.Y);

	int RandX = FMath::FRandRange(ClampedMinX, ClampedMaxX);
	int RandY = FMath::FRandRange(ClampedMinY, ClampedMaxY);

	return FIntVector(RandX, RandY, 0);
}

// Called every frame
void AMM_GridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

