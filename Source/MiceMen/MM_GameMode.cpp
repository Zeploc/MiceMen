// Copyright Alex Coultas, Mice Men Example Project


#include "MM_GameMode.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

#include "MM_PlayerController.h"
#include "Grid/MM_GridManager.h"
#include "Grid/MM_WorldGrid.h"
#include "MM_GameViewPawn.h"

AMM_GameMode::AMM_GameMode()
{	
	GridManagerClass = AMM_GridManager::StaticClass();
	PlayerControllerClass = AMM_PlayerController::StaticClass();
	DefaultPawnClass = AMM_GameViewPawn::StaticClass();
}

class AMM_GridManager* AMM_GameMode::GetGridManager()
{
	return GridManager;
}

void AMM_GameMode::BeginPlay()
{
	Super::BeginPlay();

	SetupGridManager();
}

bool AMM_GameMode::SetupGridManager()
{
	if (!GetWorld())
		return false;
	
	// Check valid manager class
	if (!GridManagerClass)
		GridManagerClass = AMM_GridManager::StaticClass();

	FIntVector2D NewGridSize = DefaultGridSize;

	// Default Spawn grid at world zero
	FTransform SpawnTransform = FTransform::Identity;

	// Try find GridTransform for overrides such as transform and grid size
	AActor* FoundWorldGrid = UGameplayStatics::GetActorOfClass(GetWorld(), AMM_WorldGrid::StaticClass());
	if (AMM_WorldGrid* WorldGrid = Cast<AMM_WorldGrid>(FoundWorldGrid))
	{
		SpawnTransform = WorldGrid->GetActorTransform();
		NewGridSize = WorldGrid->GridSize;
	}


	// Spawn Grid manager
	GridManager = GetWorld()->SpawnActorDeferred<AMM_GridManager>(GridManagerClass, SpawnTransform);
	GridManager->SetupGrid(NewGridSize);
	UGameplayStatics::FinishSpawningActor(GridManager, SpawnTransform);
	if (!GridManager)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn Grid Manager!"));
		return false;
	}

	return true;
}
