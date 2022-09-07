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
	PopulateGrid();
}

void AMM_GridManager::SetupGrid()
{
	for (int x = 0; x < GridSize.X; x++)
	{
		TArray<UMM_GridInfo*> NewGridCollumn;
		Grid.Add(NewGridCollumn);
		for (int y = 0; x < GridSize.Y; y++)
		{
			UMM_GridInfo* NewGridObject = NewObject<UMM_GridInfo>(UMM_GridInfo::StaticClass());
			if (!NewGridObject)
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to create Grid Object!"));
				continue;
			}
			NewGridObject->SetupGridInfo(FIntVector(x, y, 0));
			NewGridCollumn.Add(NewGridObject);
		}
	}
}

void AMM_GridManager::PopulateGrid()
{
	for (int x = 0; x < Grid.Num(); x++)
	{
		for (int y = 0; x < Grid[x].Num(); y++)
		{
			FTransform GridElementTransform = GetActorTransform();
			FVector NewRelativeLocation = FVector::Zero();
			NewRelativeLocation.X -= (GridSize.X / 2) * GridElementWidth;
			NewRelativeLocation.X += x * GridElementWidth;
			NewRelativeLocation.Y += y * GridElementHeight;
			// NEEDS TO BE RELATIVE LOCATION TO TRANSFORM
			GridElementTransform.SetLocation(NewRelativeLocation);
			if (FMath::RandBool())
			{
				AMM_GridBlock* NewGridBlock = GetWorld()->SpawnActor<AMM_GridBlock>(GridBlockClass, GridElementTransform);
				Grid[x][y]->SetGridBlock(NewGridBlock);
			}
		}
	}
}

// Called every frame
void AMM_GridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

