// Copyright Alex Coultas, Mice Men Example Project


#include "Gameplay/MM_Mouse.h"

#include "Kismet/GameplayStatics.h"

#include "Grid/MM_GridManager.h"
#include "Base/MM_GameMode.h"
#include "MiceMen.h"

// Sets default values
AMM_Mouse::AMM_Mouse()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMM_Mouse::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void AMM_Mouse::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMM_Mouse::SetupMouse(ETeam _Team, FIntVector2D& _FinalGridCoordinates)
{
	CurrentTeam = _Team;

	TArray<FIntVector2D> ValidPath = GetMovementPath();
	_FinalGridCoordinates = ValidPath.Last();
	Coordinates = _FinalGridCoordinates;
}

void AMM_Mouse::BN_StartMovement_Implementation(const TArray<FVector>& _Path)
{
	// Default behavior, should be overridden
	MovementEndDelegate.Broadcast(this);
	SetActorLocation(_Path.Last());
}

bool AMM_Mouse::AttemptPerformMovement()
{
	// Move Mouse to next position
	FIntVector2D FinalPosition;
	bool bSuccessfulMovement = BeginMove(FinalPosition);
	if (bSuccessfulMovement)
	{
		ProcessUpdatedPosition(FinalPosition);
	}
	else
	{
		// Mouse didn't move, movement complete
		MovementEndDelegate.Broadcast(this);
	}

	return bSuccessfulMovement;
}

bool AMM_Mouse::BeginMove(FIntVector2D& _FinalPosition)
{
	// Calculate Path
	TArray<FIntVector2D> ValidPath = GetMovementPath();
	_FinalPosition = ValidPath.Last();

	// If there is no new position/Path, go to next mouse
	if (Coordinates == _FinalPosition)
	{
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::MoveMouse | Mouse staying at %s"), *_FinalPosition.ToString());
		return false;
	}

#if !UE_BUILD_SHIPPING
	DebugPath(ValidPath);
#endif

	// TODO: Need to account for a second team movement then opens movement for the previously moved team?

	// If test mode, instantly move mouse
	if (MMGameMode->GetCurrentGameType() == EGameType::E_TEST)
	{
		SetActorLocation(GridManager->GetWorldTransformFromCoord(_FinalPosition).GetLocation());
	}
	else
	{
		// Begin movement (should fire delegate on complete)
		BN_StartMovement(GridManager->PathFromCoordToWorld(ValidPath));
	}

	return true;
}

void AMM_Mouse::ProcessUpdatedPosition(const FIntVector2D& _NewPosition)
{
	// Check if this mouse has reached the goal
	bool bReachedGoal = false;
	if (CurrentTeam == ETeam::E_TEAM_A)
	{
		// Team A reach right side of grid
		bReachedGoal = _NewPosition.X >= GridManager->GetGridSize().X - 1;
	}
	else
	{
		// Team B left side of grid
		bReachedGoal = _NewPosition.X <= 0;
	}

	// If the mouse is at the end of the grid for their team
	if (bReachedGoal)
	{
		// Mouse Completed
		GoalReached();
	}
	// Not at the end, set coordinates to end path position
	else
	{
		GridManager->MoveMouse(this, _NewPosition);
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::ProcessMouse | Mouse New position for team %i at %s"), CurrentTeam, *_NewPosition.ToString());
	}
}

TArray<FIntVector2D> AMM_Mouse::GetMovementPath() const
{
	// Setup initial variables
	TArray<FIntVector2D> Path = { Coordinates };
	FIntVector2D LastPosition = Coordinates;

	EDirection Direction = GridManager->GetDirectionFromTeam(CurrentTeam);

	// Loop while valid move
	bool bHasMove = true;
	while (bHasMove)
	{
		// Set initial valid move to false
		bHasMove = false;
		// Start at last position
		FIntVector2D NewPosition = LastPosition;
		// If valid move down, has move can be set to true
		// Store new position before ahead check
		if (GridManager->FindFreeSlotBelow(NewPosition))
		{
			Path.Add(NewPosition);
			bHasMove = true;
		}
		// If valid move ahead, has move can be set to true
		if (GridManager->FindFreeSlotAhead(NewPosition, Direction))
		{
			bHasMove = true;
		}

		// If a valid move was found, set last position and store in path
		// Otherwise will exit the while loop, having reached the final position
		if (bHasMove)
		{
			LastPosition = NewPosition;
			// Adds position to end if it was not added (slot was found from ahead check)
			if (Path.Last() != NewPosition)
			{
				Path.Add(NewPosition);
			}
		}
	}
	return Path;
}

void AMM_Mouse::GoalReached()
{
	bGoalReached = true;
	BI_OnGoalReached();

	GridManager->RemoveMouse(this);
	UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::MouseGoalReached | Mouse reached goal for team %i at %s"), CurrentTeam, *Coordinates.ToString());

	// Check stalemate
	if (MMGameMode)
	{
		MMGameMode->CheckStalemateMice();
	}
}

// ################################ Debugging ################################

void AMM_Mouse::DebugPath(TArray<FIntVector2D> ValidPath) const
{
	auto colour = FLinearColor::MakeRandomColor();
	for (int i = 0; i < ValidPath.Num(); i++)
	{
		FVector BoxLocation = GridManager->GetWorldTransformFromCoord(ValidPath[i]).GetLocation();
		BoxLocation.Z += GridManager->GridElementHeight / 2.0;
		FVector BoxBounds = FVector(40 * ((float)i / (float)10) + 5);

		UKismetSystemLibrary::DrawDebugBox(GetWorld(), BoxLocation, BoxBounds, colour, FRotator::ZeroRotator, 5, 3);
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::ProcessedMouse | Current path %i: %s"), i, *ValidPath[i].ToString());
	}
}