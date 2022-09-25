// Copyright Alex Coultas, Mice Men Example Project

#include "Player/MM_PlayerController.h"

#include "Player/MM_GameViewPawn.h"
#include "Base/MM_GameMode.h"
#include "Gameplay/MM_ColumnControl.h"
#include "Grid/MM_GridManager.h"
#include "MiceMen.h"
#include "Gameplay/MM_Mouse.h"

AMM_PlayerController::AMM_PlayerController()
{
	bShowMouseCursor = true;
}

void AMM_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	MMGameMode = GetWorld()->GetAuthGameMode<AMM_GameMode>();
}

void AMM_PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	MMPawn = Cast<AMM_GameViewPawn>(InPawn);
}

void AMM_PlayerController::SetupPlayer(const ETeam InTeam)
{
	CurrentTeam = InTeam;
}

void AMM_PlayerController::SetAsAI()
{
	bIsAI = true;
	if (MMPawn && MMPawn->IsTurnActive())
	{
		TakeAITurn();
	}
}

void AMM_PlayerController::ClearAI()
{
	bIsAI = false;
}

void AMM_PlayerController::BeginTurn()
{
	if (MMPawn)
	{
		MMPawn->BeginTurn();
	}
}

bool AMM_PlayerController::TakeAITurn()
{
	// Check if player is an AI
	if (!bIsAI)
	{
		return false;
	}

	bool bTurnSuccess = false;
	if (MMGameMode && MMGameMode->GetCurrentAIDifficulty() == EAIDifficulty::E_ADVANCED)
	{
		bTurnSuccess = TakeAdvancedAITurn();
	}
	else
	{
		bTurnSuccess = TakeRandomAITurn();
	}

	return bTurnSuccess;
}

bool AMM_PlayerController::PerformColumnAIMovement(AMM_ColumnControl* Column, const int Direction) const
{
	if (!Column)
	{
		return false;
	}

	// Get new location of column
	FVector NewLocation = Column->GetActorLocation();
	NewLocation.Z += Direction * MMGameMode->GetGridManager()->GridElementHeight;

	// Grab and move to determined location
	// Note: Don't use pawn grab since its not based on mouse/input
	if (!Column->BeginGrab())
	{
		return false;
	}

	UE_LOG(MiceMenEventLog, Log, TEXT("AMM_GameViewPawn::TakeRandomAITurn | Chosen direction %i for column %i"),
	       Direction, Column->GetColumnIndex());
	Column->UpdatePreviewLocation(NewLocation);

	// If testing mode, instantly move column
	if (MMGameMode && MMGameMode->GetCurrentGameType() == EGameType::E_TEST)
	{
		Column->SetActorLocation(NewLocation);
	}

	return true;
}

bool AMM_PlayerController::TakeRandomAITurn() const
{
	if (!MMPawn)
	{
		return false;
	}
	if (!MMGameMode->GetGridManager())
	{
		return false;
	}

	TArray<AMM_ColumnControl*> CurrentColumnControls = MMPawn->GetCurrentColumnControls();

	// Find random column
	const int RandomIndex = FMath::RandRange(0, CurrentColumnControls.Num() - 1);
	AMM_ColumnControl* CurrentColumn = CurrentColumnControls[RandomIndex];

	// Find random direction
	const int RandomDirection = FMath::RandBool() ? 1 : -1;

	if (!PerformColumnAIMovement(CurrentColumn, RandomDirection))
	{
		return false;
	}

	OnAITurnComplete.Broadcast(CurrentColumn);
	return true;
}

bool AMM_PlayerController::TakeAdvancedAITurn() const
{
	if (!MMPawn)
	{
		return false;
	}
	if (!MMGameMode->GetGridManager())
	{
		return false;
	}

	TArray<AMM_ColumnControl*> CurrentColumnControls = MMPawn->GetCurrentColumnControls();
	TArray<AMM_Mouse*> TeamMice = MMGameMode->GetGridManager()->GetMiceTeams()[CurrentTeam];

	int HighestScore = 0;
	int BestColumn = -1;
	
	// Having up first means the default state when no optimal moves exist will be to move the column upwards
	EDirection BestColumnDirection = EDirection::E_UP;

	for (const AMM_ColumnControl* ColumnControl : CurrentColumnControls)
	{
		const int ColumnIndex = ColumnControl->GetColumnIndex();
		TArray<EDirection> Directions = {EDirection::E_UP, EDirection::E_DOWN};
		for (const EDirection Direction : Directions)
		{
			int CurrentScore = 0;

			// Find direction to move back column
			EDirection OppositeDirection = EDirection::E_UP;
			if (Direction == EDirection::E_UP)
			{
				OppositeDirection = EDirection::E_DOWN;
			}

			// Move column to test for score
			AMM_GridElement* LastGridElement;
			MMGameMode->GetGridManager()->AdjustColumnInGridObject(ColumnIndex, Direction, LastGridElement);

			// Find out score ie amount of movement for all mice
			for (const AMM_Mouse* Mouse : TeamMice)
			{
				// Minus one as by default the path has the initial position (no movement)
				CurrentScore += FMath::Max(0, Mouse->GetMovementPath().Num() - 1);
			}

			// Return the column back
			MMGameMode->GetGridManager()->AdjustColumnInGridObject(ColumnIndex, OppositeDirection, LastGridElement);

			// Compare score
			if (CurrentScore > HighestScore)
			{
				HighestScore = CurrentScore;
				BestColumn = ColumnIndex;
				BestColumnDirection = Direction;
			}
		}
	}

	// No column movement was found
	if (BestColumn < 0)
	{
		// Default to random movement
		return TakeRandomAITurn();
	}

	// Apply column movement
	AMM_ColumnControl* CurrentColumn = MMGameMode->GetGridManager()->GetColumnControls()[BestColumn];
	const int Direction = BestColumnDirection == EDirection::E_UP ? 1 : -1;

	if (!PerformColumnAIMovement(CurrentColumn, Direction))
	{
		return false;
	}

	OnAITurnComplete.Broadcast(CurrentColumn);
	return true;
}

void AMM_PlayerController::TurnEnded()
{
}
