// Copyright Alex Coultas, Mice Men Example Project


#include "Player/MM_PlayerController.h"

#include "Player/MM_GameViewPawn.h"
#include "Base/MM_GameMode.h"
#include "Gameplay/MM_ColumnControl.h"
#include "Grid/MM_GridManager.h"
#include "MiceMen.h"

AMM_PlayerController::AMM_PlayerController()
{
	bShowMouseCursor = true;
}

void AMM_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	MMGameMode = GetWorld()->GetAuthGameMode<AMM_GameMode>();
}

void AMM_PlayerController::OnPossess(APawn* _Pawn)
{
	Super::OnPossess(_Pawn);

	MMPawn = Cast<AMM_GameViewPawn>(_Pawn);
}

void AMM_PlayerController::SetupPlayer(ETeam _Team)
{
	CurrentTeam = _Team;
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

bool AMM_PlayerController::TakeRandomAITurn()
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
	
	if (!CurrentColumn)
	{
		return false;
	}
	
	// Get new location of column
	FVector NewLocation = CurrentColumn->GetActorLocation();
	NewLocation.Z += RandomDirection * MMGameMode->GetGridManager()->GridElementHeight;

	// Grab and move to determined location
	// Note: Don't use pawn grab since its not based on mouse/input
	CurrentColumn->BeginGrab();
	
	UE_LOG(MiceMenEventLog, Log, TEXT("AMM_GameViewPawn::TakeRandomAITurn | Chosen direction %i for column %i"), RandomDirection, RandomIndex);
	CurrentColumn->UpdatePreviewLocation(NewLocation);
	
	// If testing mode, instantly move column
	if (MMGameMode && MMGameMode->GetCurrentGameType() == EGameType::E_TEST)
	{
		CurrentColumn->SetActorLocation(NewLocation);
	}

	OnAITurnComplete.Broadcast(CurrentColumn);
	return true;	
}

bool AMM_PlayerController::TakeAdvancedAITurn()
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

	
	AMM_ColumnControl* CurrentColumn = CurrentColumnControls[0];
	
	// OnAITurnComplete.Broadcast(CurrentColumn);
	return true;	
}

void AMM_PlayerController::TurnEnded()
{

}
