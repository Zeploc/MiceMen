// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Grid/IntVector2D.h"
#include "MM_GameMode.generated.h"

class APlayerController;
class AMM_PlayerController;
class AMM_GridManager;
class AMM_Mouse;
class ULocalPlayer;

/**
 * Control for the main gameplay, grid systems and players
 */
UCLASS()
class MICEMEN_API AMM_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMM_GameMode();

	UFUNCTION(BlueprintPure)
		AMM_GridManager* GetGridManager();

	void PlayerTurnComplete(AMM_PlayerController* _Player);

	/**  Described as stalemate, when only 1 mouse exists per team */
	void CheckStalemateMice();

	/**  Find winning team in a stalemate situation */
	int GetWinningStalemateTeam();

	AMM_PlayerController* GetCurrentPlayer() { return CurrentPlayer; }

	/**
	 * Sets up team information in the gamemode
	 * such as initial points.
	 * 
	 * @param _iTeam - team to setup
	 */
	void AddTeam(int _iTeam);

	/**
	 * When a mouse reaches the end and should score a point.
	 * 
	 * @param _Mouse - the mouse that completed
	 * @return true if the game has reached a win condition
	 */
	bool MouseCompleted(AMM_Mouse* _Mouse);

	/**
	 * Checks if a team has completed all their mice.
	 * @return true if a team has one and the game should end
	 */
	bool CheckWinCondition();

protected:
	virtual void BeginPlay() override;

	void BeginGame();

	/** Creates grid and basic setup */
	bool SetupGridManager();

	virtual void PostLogin(APlayerController* _NewPlayer) override;

	void SwitchTurns(AMM_PlayerController* _Player);

	UFUNCTION(BlueprintImplementableEvent)
	void BI_OnSwitchTurns(AMM_PlayerController* _Player);

	void TeamWon(int _iTeam);
	UFUNCTION(BlueprintImplementableEvent)
		void BI_OnTeamWon(int _iTeam);

public:
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<AMM_GridManager> GridManagerClass;

	/**  The amount of turns the game will last for when entered stalemate (one mouse per team) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int StalemateTurns = 8;

	/**  The starting number of mice on each team */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int InitialMiceCount = 12;

protected:
	UPROPERTY(BlueprintReadOnly)
	class AMM_GridManager* GridManager;

	UPROPERTY(EditDefaultsOnly)
		FIntVector2D DefaultGridSize = FIntVector2D(19, 13);

	UPROPERTY(BlueprintReadOnly)
		TArray<AMM_PlayerController*> AllPlayers;

	UPROPERTY(BlueprintReadOnly)
		AMM_PlayerController* CurrentPlayer;

	/** The current team points, Team ID to number of points */
	UPROPERTY(BlueprintReadOnly)
	TMap<int, int> TeamPoints;

	/* Stored local player to switch controller*/
	ULocalPlayer* FirstLocalPlayer;

	/** When a stalemate is entered, this value will count up per turn */
	UPROPERTY(BlueprintReadOnly)
		int StalemateCount = -1;
};

