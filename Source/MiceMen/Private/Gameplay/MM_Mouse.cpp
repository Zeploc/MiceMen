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
	const bool bSuccessfulMovement = BeginMove(FinalPosition);
	
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

	// If there is no new position/Path, no movement needed
	if (Coordinates == _FinalPosition)
	{
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_Mouse::BeginMove | Mouse staying at %s"), *_FinalPosition.ToString());
		return false;
	}

#if !UE_BUILD_SHIPPING
	DisplayDebugPath(ValidPath);
#endif

	// TODO: Need to account for a second team movement then opens movement for the previously moved team?

	// If test mode, instantly move mouse
	if (MMGameMode->GetCurrentGameType() == EGameType::E_TEST)
	{
		SetActorLocation(GridManager->CoordToWorldTransform(_FinalPosition).GetLocation());
	}
	else
	{
		// Begin movement (should fire delegate on complete)
		BN_StartMovement(GridManager->PathCoordToWorld(ValidPath));
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
		// Score a point and clear the mouse
		GoalReached();
	}
	// Not at the end, set coordinates to end path position
	else
	{
		GridManager->SetMousePosition(this, _NewPosition);
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_Mouse::ProcessUpdatedPosition | Mouse New position for team %i at %s"), CurrentTeam, *_NewPosition.ToString());
	}
}

TArray<FIntVector2D> AMM_Mouse::GetMovementPath() const
{
	// Setup initial variables
	TArray<FIntVector2D> Path = { Coordinates };
	FIntVector2D LastPosition = Coordinates;

	const EDirection Direction = GridManager->GetDirectionFromTeam(CurrentTeam);

	// Loop while valid move
	bool bHasMove = true;
	while (bHasMove)
	{
		// Set initial valid move to false
		bHasMove = false;
		// Start at last position
		FIntVector2D NewPosition = LastPosition;
		
		// If valid move down, bHasMove can be set to true
		if (GridManager->FindFreeSlotBelow(NewPosition))
		{
			// Store new position before ahead check
			Path.Add(NewPosition);
			bHasMove = true;
		}
		// If valid move ahead, bHasMove can be set to true
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
	UE_LOG(MiceMenEventLog, Display, TEXT("AMM_Mouse::GoalReached | Mouse reached goal for team %i at %s"), CurrentTeam, *Coordinates.ToString());

	// Check stalemate
	if (MMGameMode)
	{
		MMGameMode->CheckForStalemate();
	}
}

// ################################ Debugging ################################

void AMM_Mouse::DisplayDebugPath(const TArray<FIntVector2D>& _ValidPath) const
{
	const FLinearColor Colour = FLinearColor::MakeRandomColor();
	for (int i = 0; i < _ValidPath.Num(); i++)
	{
		FVector BoxLocation = GridManager->CoordToWorldTransform(_ValidPath[i]).GetLocation();
		// Add half height to be in center of griid
		BoxLocation.Z += GridManager->GridElementHeight / 2.0;
		constexpr float StartingSize = 5.0f;
		// Arbitrary visual size increase up to 10 movements
		const float AmountThroughPath = static_cast<float>(i) / static_cast<float>(10);
		const FVector BoxBounds = FVector(StartingSize + AmountThroughPath * 40.0f);

		UKismetSystemLibrary::DrawDebugBox(GetWorld(), BoxLocation, BoxBounds, Colour, FRotator::ZeroRotator, 5, 3);
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_Mouse::DisplayDebugPath | Current path %i: %s"), i, *_ValidPath[i].ToString());
	}
}