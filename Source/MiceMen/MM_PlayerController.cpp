// Copyright Alex Coultas, Mice Men Example Project


#include "MM_PlayerController.h"

#include "MM_GameViewPawn.h"
#include "MM_GameMode.h"

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

void AMM_PlayerController::SetupPlayer(int _Team)
{
	CurrentTeam = _Team;
}

void AMM_PlayerController::BeginTurn()
{
	MMPawn->BeginTurn();
}

void AMM_PlayerController::TurnEnded()
{
	/*if (MMGameMode)
		MMGameMode->PlayerTurnComplete(this);*/
}
