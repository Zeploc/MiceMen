// Copyright Alex Coultas, Mice Men Example Project


#include "MM_GameMode.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

#include "MM_PlayerController.h"
#include "Grid/MM_GridManager.h"
#include "Grid/MM_WorldGrid.h"
#include "MM_GameViewPawn.h"
#include "MiceMen.h"

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

void AMM_GameMode::PlayerTurnComplete(class AMM_PlayerController* _Player)
{
	// Was not the players current turn
	if (_Player != CurrentPlayer)
	{
		UE_LOG(MiceMenEventLog, Warning, TEXT("AMM_GameViewPawn::TurnEnded | Attempted to end turn for incorrect player %i:%s when current player is %i:%s"), _Player->GetCurrentTeam(), *_Player->GetName(), CurrentPlayer->GetCurrentTeam(), *CurrentPlayer->GetName());
		return;
	}
	
	// Get next player, wrap around if at the end
	int NextPlayer = AllPlayers.Find(_Player);
	NextPlayer++;
	if (NextPlayer > AllPlayers.Num() - 1)
		NextPlayer = 0;

	UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GameMode::PlayerTurnComplete | Completed players turn with %i as %s"), CurrentPlayer->GetCurrentTeam(), *CurrentPlayer->GetName());
	CurrentPlayer->TurnEnded();

	SwitchTurns(AllPlayers[NextPlayer]);

}

void AMM_GameMode::BeginPlay()
{
	Super::BeginPlay();

	//SetupGridManager();
}

void AMM_GameMode::BeginGame()
{
	if (AllPlayers.Num() < 2)
		return;

	SetupGridManager();

	SwitchTurns(AllPlayers[0]);
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

void AMM_GameMode::PostLogin(APlayerController* _NewPlayer)
{
	Super::PostLogin(_NewPlayer);

	if (AMM_PlayerController* MMController = Cast<AMM_PlayerController>(_NewPlayer))
	{
		// Current length is next index
		MMController->SetupPlayer(AllPlayers.Num());
		AllPlayers.Add(MMController);

		// Currently only requires two players
		if (AllPlayers.Num() >= 2)
		{
			BeginGame();
		}
	}
}

void AMM_GameMode::SwitchTurns(AMM_PlayerController* _Player)
{
	if (!_Player)
		return;


	// Store local player
	ULocalPlayer* LocalPlayer = _Player->GetLocalPlayer();
	if (CurrentPlayer)
		LocalPlayer = CurrentPlayer->GetLocalPlayer();

	// Begin player turn
	CurrentPlayer = _Player;
	UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GameMode::SwitchTurns | Switching player to %i as %s"), CurrentPlayer->GetCurrentTeam(), *CurrentPlayer->GetName());
	CurrentPlayer->BeginTurn();

	// Since this is local, set the new player to active input
	// Note: In a network situation this would not be necessary as each client has their own input
	// And would send events to the server
	LocalPlayer->SwitchController(CurrentPlayer);


	BI_OnSwitchTurns(CurrentPlayer);
}
