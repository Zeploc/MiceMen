// Copyright Alex Coultas, Mice Men Example Project


#include "Base/MM_GameMode.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

#include "Player/MM_PlayerController.h"
#include "Grid/MM_GridManager.h"
#include "Grid/MM_WorldGrid.h"
#include "Player/MM_GameViewPawn.h"
#include "MiceMen.h"
#include "Gameplay/MM_Mouse.h"

AMM_GameMode::AMM_GameMode()
{	
	GridManagerClass = AMM_GridManager::StaticClass();
	PlayerControllerClass = AMM_PlayerController::StaticClass();
	DefaultPawnClass = AMM_GameViewPawn::StaticClass();
}

AMM_GridManager* AMM_GameMode::GetGridManager()
{
	return GridManager;
}

void AMM_GameMode::BeginPlay()
{
	Super::BeginPlay();

	// Create second local player
	SecondLocalPlayerController = UGameplayStatics::CreatePlayer(GetWorld());
}

void AMM_GameMode::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Not concerned with cleanup if the application/process is ending as that is handled internally
	if (EndPlayReason == EEndPlayReason::EndPlayInEditor || EndPlayReason == EEndPlayReason::Quit)
	{
		return;
	}

	// Reset and clear
	CleanupGame();

	// Remove second player (local player persists through world switches)
	if (SecondLocalPlayerController)
	{
		UGameplayStatics::RemovePlayer(SecondLocalPlayerController, true);
	}
}

void AMM_GameMode::RestartGame()
{
	CleanupGame();

	BI_OnGameRestarted();

	BeginGame(CurrentGameType, CurrentAIDifficulty);
}

void AMM_GameMode::CleanupGame()
{
	// Clear grid manager
	if (GridManager)
	{
		GridManager->Destroy();
		GridManager = nullptr;
	}
	// Change back to first local player
	if (FirstLocalPlayer && AllPlayers.IsValidIndex(0))
	{
		FirstLocalPlayer->SwitchController(AllPlayers[0]);
	}

	// Restore variables
	StalemateCount = -1;
	TeamPoints.Empty();
}

void AMM_GameMode::GameReady()
{
	BI_OnGameReady();
}

void AMM_GameMode::BeginGame(EGameType InGameType, EAIDifficulty InAIDifficulty)
{
	// Not all players joined, cannot begin
	if (AllPlayers.Num() < 2)
	{
		return;
	}
	// No valid game type set, cannot begin
	if (InGameType == EGameType::E_NONE || InGameType == EGameType::E_MAX)
	{
		UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GameMode::BeginGame | No gametype selected!"));
		return;
	}

	// Reset and Restart players
	for (AMM_PlayerController* PlayerController : AllPlayers)
	{
		if (!PlayerController)
		{
			continue;
		}
		PlayerController->ClearAI();
		if (PlayerController->GetPawn())
		{
			PlayerController->GetPawn()->Destroy();
		}
		PlayerController->UnPossess();
		RestartPlayer(PlayerController);
	}

	// Setup
	SetupGridManager();
	CheckForStalemate();

	// Setup game type
	CurrentGameType = InGameType;
	CurrentAIDifficulty = InAIDifficulty;
	switch (CurrentGameType)
	{
	case EGameType::E_PVP:
		break;
	case EGameType::E_TEST:
	{
		// Fall through (set players to AI)
	}
	case EGameType::E_AIVAI:
	{
		if (AllPlayers.IsValidIndex(0))
		{
			AllPlayers[0]->SetAsAI();
		}
		// Fall through to set second player to AI
	}
	case EGameType::E_PVAI:
	{
		if (AllPlayers.IsValidIndex(1))
		{
			AllPlayers[1]->SetAsAI();
		}

		break;
	}
	case EGameType::E_SANDBOX:
		break;
	default:
		break;
	}

	// Start random players turn
	const int IntialPlayer = FMath::RandRange(0, AllPlayers.Num() - 1);
	SwitchTurnToPlayer(AllPlayers[IntialPlayer]);
	
	BI_OnGameBegun();
}

bool AMM_GameMode::SetupGridManager()
{
	if (!GetWorld())
	{
		return false;
	}
	
	// Initially set grid size to game mode default
	FIntVector2D GridSize = DefaultGridSize;

	// Initially Spawn grid at world zero
	FTransform SpawnTransform = FTransform::Identity;

	// Try find World Grid for overrides such as transform and grid size
	AActor* FoundWorldGrid = UGameplayStatics::GetActorOfClass(GetWorld(), AMM_WorldGrid::StaticClass());
	if (const AMM_WorldGrid* WorldGrid = Cast<AMM_WorldGrid>(FoundWorldGrid))
	{
		SpawnTransform = WorldGrid->GetActorTransform();
		GridSize = WorldGrid->GridSize;
	}

	// Check valid manager class
	if (!GridManagerClass)
	{
		// Set to static class if no valid class set
		GridManagerClass = AMM_GridManager::StaticClass();
	}
	UE_LOG(LogTemp, Display, TEXT("AMM_GameMode::SetupGridManager | Setting up Grid Manager with class %s"), *GridManagerClass->GetName());
	
	// Spawn Grid manager
	GridManager = GetWorld()->SpawnActorDeferred<AMM_GridManager>(GridManagerClass, SpawnTransform);
	GridManager->SetupGridVariables(GridSize, this);
	UGameplayStatics::FinishSpawningActor(GridManager, SpawnTransform);
	GridManager->RebuildGrid(InitialMiceCount);

	if (!GridManager)
	{
		UE_LOG(LogTemp, Error, TEXT("AMM_GameMode::SetupGridManager | Failed to spawn Grid Manager!"));
		return false;
	}

	return true;
}

void AMM_GameMode::SwitchToTestMode()
{
	CurrentGameType = EGameType::E_TEST;

	// Change both players to AI
	for (AMM_PlayerController* PlayerController : AllPlayers)
	{
		if (PlayerController)
		{
			PlayerController->SetAsAI();
		}
	}
}

void AMM_GameMode::EndGame()
{
	CleanupGame();
	
	BI_OnGameEnded();
}

void AMM_GameMode::AddTeam(ETeam Team)
{
	TeamPoints.Add(Team, 0);
}

void AMM_GameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AMM_PlayerController* MMController = Cast<AMM_PlayerController>(NewPlayer);
	// Check player controller is valid and correct type
	if (!MMController)
	{
		return;
	}

	ETeam NewPlayerTeam;
	
	// First player is team A
	if (AllPlayers.Num() == 0)
	{
		NewPlayerTeam = ETeam::E_TEAM_A;

		// Store first local player
		FirstLocalPlayer = MMController->GetLocalPlayer();
	}
	// Second player joins team B
	else
	{
		NewPlayerTeam = ETeam::E_TEAM_B;
	}

	// Set up player as selected team
	MMController->SetupPlayer(NewPlayerTeam);
	AllPlayers.Add(MMController);

	// Currently only requires two players to begin
	if (AllPlayers.Num() >= 2)
	{
		GameReady();
	}
}

void AMM_GameMode::SwitchTurnToPlayer(AMM_PlayerController* Player)
{
	if (!Player)
	{
		return;
	}
	
	// Store new player as current
	CurrentPlayerController = Player;
	
	// Since this is local, set the first local player to the new player to enable input
	// Note: In a network or split-screen situation, this would not be necessary as each client has their own input
	// And would send events to the server
	if (FirstLocalPlayer)
	{
		FirstLocalPlayer->SwitchController(CurrentPlayerController);
	}	
	
	// Begin player turn
	UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GameMode::SwitchTurnToPlayer | Switching player to %i as %s"), CurrentPlayerController->GetCurrentTeam(), *CurrentPlayerController->GetName());
	CurrentPlayerController->BeginTurn();

	BI_OnSwitchTurns(CurrentPlayerController);
}

void AMM_GameMode::ProcessTurnComplete(AMM_PlayerController* Player)
{
	// Was not the player's current turn
	if (Player != CurrentPlayerController)
	{
		UE_LOG(MiceMenEventLog, Warning, TEXT("AMM_GameMode::ProcessTurnComplete | Attempted to end turn for incorrect player %i:%s when current player is %i:%s"), Player->GetCurrentTeam(), *Player->GetName(), CurrentPlayerController->GetCurrentTeam(), *CurrentPlayerController->GetName());
		return;
	}

	// End turn for current player
	UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GameMode::ProcessTurnComplete | Completed player's turn %s as %i"), *CurrentPlayerController->GetName(), CurrentPlayerController->GetCurrentTeam());
	CurrentPlayerController->TurnEnded();
	
	// If stalemate is active, increase counter for turn taken
	if (StalemateCount >= 0)
	{
		// Turn taken, increase count
		StalemateCount++;

		// Check if stalemate turn count reached
		if (StalemateCount >= StalemateTurns)
		{
			int DistanceWonBy = 0;
			const ETeam WinningTeam = GetWinningStalemateTeam(DistanceWonBy);
			FString WinningMessage = "Stalemate reached";
			if (WinningTeam != ETeam::E_NONE)
			{
				WinningMessage = FString::Printf(TEXT("Stalemate reached with remaining mouse more ahead by %i"), DistanceWonBy);
			}
			// Find winning stalemate team and end the game
			TeamWon(WinningTeam, WinningMessage);
			return;
		}
	}

	// Get next player, wrap around if at the end
	int NextPlayer = AllPlayers.Find(Player);
	NextPlayer++;
	if (NextPlayer > AllPlayers.Num() - 1)
	{
		NextPlayer = 0;
	}

	// Switch to next player
	SwitchTurnToPlayer(AllPlayers[NextPlayer]);
}

void AMM_GameMode::CheckForStalemate()
{
	// If more than 0, already in stalemate mode
	if (StalemateCount >= 0)
	{
		return;
	}

	// Check with grid manager if should currently be in stalemate 
	if (GetGridManager() && GridManager->IsStalemate())
	{
		// Enter stalemate mode and begin counting turns
		StalemateCount = 0;
	}	
}

ETeam AMM_GameMode::GetWinningStalemateTeam(int& DistanceWonBy) const
{
	if (GridManager)
	{
		return GridManager->GetWinningStalemateTeam(DistanceWonBy);
	}

	UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GameMode::GetWinningStalemateTeam | Failed to get winning stalemate team, GridManager not valid!"));
	return ETeam::E_NONE;
}

void AMM_GameMode::ForceEndNoMoves()
{
	ETeam WinningTeam = ETeam::E_NONE;
	FString WinningReason = "No more moves remaining";
	if (TeamPoints[ETeam::E_TEAM_A] != TeamPoints[ETeam::E_TEAM_B])
	{
		if (TeamPoints[ETeam::E_TEAM_A] > TeamPoints[ETeam::E_TEAM_B])
		{
			WinningTeam = ETeam::E_TEAM_A;
		}
		else
		{
			WinningTeam = ETeam::E_TEAM_B;
		}
		WinningReason = "No more mice can complete, winning team has more completed mice";
	}

	TeamWon(WinningTeam, WinningReason);
}

void AMM_GameMode::AddScore(ETeam Team)
{
	// Increment score by 1, will set score if the team's score doesn't exists
	TeamPoints.Add(Team, GetTeamScore(Team) + 1);

	// If a team has the winning number of points
	if (HasTeamWon(Team))
	{
		TeamWon(Team, "Completed all mice");
	}
}

int AMM_GameMode::GetTeamScore(const ETeam Team) const
{
	int CurrentScore = 0;
	if (const int* FoundScore = TeamPoints.Find(Team))
	{
		CurrentScore = *FoundScore;
	}
	return CurrentScore;
}

bool AMM_GameMode::HasTeamWon(ETeam TeamToCheck) const
{
	// If a team has scored all their mice
	if (TeamPoints[TeamToCheck] >= InitialMiceCount)
	{
		return true;
	}
	
	// Team has won yet
	return false;
}

void AMM_GameMode::TeamWon(ETeam Team, const FString& Reason)
{
	// Check valid team
	if (Team == ETeam::E_MAX)
	{
		UE_LOG(LogTemp, Error, TEXT("No Valid Team won!"));
		return;
	}
	
	BI_OnTeamWon(Team, Reason);
}