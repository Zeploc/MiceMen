// Copyright Alex Coultas, Mice Men Example Project


#include "Player/MM_PlayerController.h"

#include "Player/MM_GameViewPawn.h"
#include "Base/MM_GameMode.h"

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
		MMPawn->TakeRandomTurn();
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

void AMM_PlayerController::TurnEnded()
{

}
