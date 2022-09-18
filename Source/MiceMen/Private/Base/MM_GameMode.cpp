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

	SecondLocalPlayerController = UGameplayStatics::CreatePlayer(GetWorld());
}

void AMM_GameMode::EndPlay(EEndPlayReason::Type _EndPlayReason)
{
	Super::EndPlay(_EndPlayReason);

	// Not concerned with cleanup if the application/process is ending as that is handled internally
	if (_EndPlayReason == EEndPlayReason::EndPlayInEditor || _EndPlayReason == EEndPlayReason::Quit)
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

	BeginGame(CurrentGameType);
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

void AMM_GameMode::BeginGame(EGameType _GameType)
{
	if (AllPlayers.Num() < 2)
	{
		return;
	}
	if (_GameType == EGameType::E_NONE || _GameType == EGameType::E_MAX)
	{
		UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GameMode::BeginGame | No gametype selected!"));
		return;
	}

	// Restart players
	for (AMM_PlayerController* PlayerController : AllPlayers)
	{
		if (!PlayerController)
		{
			continue;
		}
		PlayerController->ClearAI();
		RestartPlayer(PlayerController);
	}

	// Setup
	SetupGridManager();
	CheckStalemateMice();

	// Setup game type
	CurrentGameType = _GameType;
	switch (CurrentGameType)
	{
	case EGameType::E_PVP:
		break;
	case EGameType::E_TEST:
	{
		// Fall down (set players to AI)
	}
	case EGameType::E_AIVAI:
	{
		if (AllPlayers.IsValidIndex(0))
		{
			AllPlayers[0]->SetAsAI();
		}
		// Fall down
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
	int IntialPlayer = FMath::RandRange(0, AllPlayers.Num() - 1);
	SwitchTurns(AllPlayers[IntialPlayer]);
	
	BI_OnGameBegun();
}

void AMM_GameMode::SwitchToTest()
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

void AMM_GameMode::AddTeam(int _iTeam)
{
	TeamPoints.Add(_iTeam, 0);
}

void AMM_GameMode::PostLogin(APlayerController* _NewPlayer)
{
	Super::PostLogin(_NewPlayer);

	if (AMM_PlayerController* MMController = Cast<AMM_PlayerController>(_NewPlayer))
	{
		// Current length of players is next player index
		MMController->SetupPlayer(AllPlayers.Num());
		AllPlayers.Add(MMController);

		// Store first local player
		if (MMController->GetLocalPlayer() && !FirstLocalPlayer)
		{
			FirstLocalPlayer = MMController->GetLocalPlayer();
		}

		// Currently only requires two players
		if (AllPlayers.Num() >= 2)
		{
			GameReady();
		}
	}
}

void AMM_GameMode::SwitchTurns(AMM_PlayerController* _Player)
{
	if (!_Player)
	{
		return;
	}

	// Begin player turn
	CurrentPlayer = _Player;
	UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GameMode::SwitchTurns | Switching player to %i as %s"), CurrentPlayer->GetCurrentTeam(), *CurrentPlayer->GetName());
	CurrentPlayer->BeginTurn();

	// Since this is local, set the new player to active input
	// Note: In a network or splitscreen situation this would not be necessary as each client has their own input
	// And would send events to the server
	if (FirstLocalPlayer)
	{
		FirstLocalPlayer->SwitchController(CurrentPlayer);
	}

	BI_OnSwitchTurns(CurrentPlayer);
}

void AMM_GameMode::PlayerTurnComplete(AMM_PlayerController* _Player)
{
	// Was not the player's current turn
	if (_Player != CurrentPlayer)
	{
		UE_LOG(MiceMenEventLog, Warning, TEXT("AMM_GameViewPawn::TurnEnded | Attempted to end turn for incorrect player %i:%s when current player is %i:%s"), _Player->GetCurrentTeam(), *_Player->GetName(), CurrentPlayer->GetCurrentTeam(), *CurrentPlayer->GetName());
		return;
	}

	// End turn for current player
	UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GameMode::PlayerTurnComplete | Completed player's turn %s as %i"), *CurrentPlayer->GetName(), CurrentPlayer->GetCurrentTeam());
	CurrentPlayer->TurnEnded();
	
	// If stalemate is active, increase counter for turn taken
	if (StalemateCount >= 0)
	{
		// Turn taken, increase count
		StalemateCount++;

		// Check if stalemate turn count reached
		if (StalemateCount >= StalemateTurns)
		{
			// Find winning stalemate team and end the game
			TeamWon(GetWinningStalemateTeam());
			return;
		}
	}

	// Get next player, wrap around if at the end
	int NextPlayer = AllPlayers.Find(_Player);
	NextPlayer++;
	if (NextPlayer > AllPlayers.Num() - 1)
	{
		NextPlayer = 0;
	}

	// Switch to next player
	SwitchTurns(AllPlayers[NextPlayer]);
}

void AMM_GameMode::CheckStalemateMice()
{
	// If more than 0, already in stalemate mode
	if (StalemateCount >= 0)
	{
		return;
	}

	// Check if in stalemate with grid manager
	if (GetGridManager() && GridManager->IsStalemate())
	{
		// Enter stalemate mode and begin counting turns
		StalemateCount = 0;
	}	
}

int AMM_GameMode::GetWinningStalemateTeam()
{
	if (GridManager)
	{
		return GridManager->GetWinningStalemateTeam();
	}

	UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GameMode::GetWinningStalemateTeam | Failed to get winning stalemate team, GridManager not valid!"));
	return 0;
}

void AMM_GameMode::AddScore(int _Team)
{
	// Increment score by 1, will set score if the team already exists
	TeamPoints.Add(_Team, GetTeamScore(_Team) + 1);

	// If mouse complete was winning mouse, stop processing mice
	if (HasTeamWon(_Team))
	{
		TeamWon(_Team);
	}
}

int AMM_GameMode::GetTeamScore(int _Team)
{
	int CurrentScore = 0;
	if (int* FoundScore = TeamPoints.Find(_Team))
	{
		CurrentScore = *FoundScore;
	}
	return CurrentScore;
}

bool AMM_GameMode::HasTeamWon(int _TeamToCheck)
{
	// If a team has scored all their mice
	if (TeamPoints[_TeamToCheck] >= InitialMiceCount)
	{
		return true;
	}
	
	// No team has won yet
	return false;
}


bool AMM_GameMode::SetupGridManager()
{
	if (!GetWorld())
	{
		return false;
	}
	
	// Check valid manager class
	if (!GridManagerClass)
	{
		GridManagerClass = AMM_GridManager::StaticClass();
	}

	UE_LOG(LogTemp, Display, TEXT("AMM_GameMode::SetupGridManager | Setting up Grid Manager with class %s"), *GridManagerClass->GetName());

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
	GridManager->SetupGrid(NewGridSize, this);
	UGameplayStatics::FinishSpawningActor(GridManager, SpawnTransform);
	GridManager->RebuildGrid(InitialMiceCount);

	if (!GridManager)
	{
		UE_LOG(LogTemp, Error, TEXT("AMM_GameMode::SetupGridManager | Failed to spawn Grid Manager!"));
		return false;
	}

	return true;
}



void AMM_GameMode::TeamWon(int _iTeam)
{
	if (_iTeam < -1)
	{
		UE_LOG(LogTemp, Error, TEXT("No Valid Team won!"));
		return;
	}
	BI_OnTeamWon(_iTeam);
}
