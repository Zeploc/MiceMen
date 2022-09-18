// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Grid/IntVector2D.h"
#include "Base/MM_Enums.h"
#include "MM_GameMode.generated.h"

class APlayerController;
class AMM_PlayerController;
class AMM_GridManager;
class AMM_Mouse;
class ULocalPlayer;

/**
 * Control for the main game play, grid systems and players
 */
UCLASS()
class MICEMEN_API AMM_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMM_GameMode();

#pragma region Virtual Overriden

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(EEndPlayReason::Type _EndPlayReason) override;

	/** When a new player joins, will store and check if the game is ready to start */
	virtual void PostLogin(APlayerController* _NewPlayer) override;

#pragma endregion

#pragma region Game Loop

public:
	/** Starts game with a certain game play type */
	UFUNCTION(BlueprintCallable)
		void BeginGame(EGameType _GameType);

	/** Clean up grid and restore to starting state */
	void CleanupGame();

	/** Completes the game, and cleans up all grid systems */
	UFUNCTION(BlueprintCallable)
		void EndGame();

	/**  Resets the board and players */
	UFUNCTION(BlueprintCallable)
		void RestartGame();

	/** Changes current mode to test and switches players to AI */
	UFUNCTION(BlueprintCallable)
		void SwitchToTest();

	UFUNCTION(BlueprintPure)
		EGameType GetCurrentGameType() const { return CurrentGameType; }

protected:
	/** Called when the game is ready and game play mode can be chosen */
	void GameReady();

	UFUNCTION(BlueprintImplementableEvent)
		void BI_OnGameReady();

	UFUNCTION(BlueprintImplementableEvent)
		void BI_OnGameBegun();

	UFUNCTION(BlueprintImplementableEvent)
		void BI_OnGameEnded();

	UFUNCTION(BlueprintImplementableEvent)
		void BI_OnGameRestarted();

#pragma endregion

#pragma region Grid

public:
	UFUNCTION(BlueprintPure)
		AMM_GridManager* GetGridManager();

protected:
	/** Creates grid and basic setup, uses AMM_WorldGrid to position the grid*/
	bool SetupGridManager();

#pragma endregion

#pragma region Player Turns

public:
	/**
	* Once a player's turn is complete, performs checks
	* Will either complete the game or go to the next player.
	* @param _Player the player who's turn just ended
	*/
	void PlayerTurnComplete(AMM_PlayerController* _Player);

	/** Get the player who's current turn it is */
	UFUNCTION(BlueprintPure)
		AMM_PlayerController* GetCurrentPlayer() const { return CurrentPlayer; }

protected:
	/** Will change turns to a different player */
	void SwitchTurns(AMM_PlayerController* _Player);

	UFUNCTION(BlueprintImplementableEvent)
		void BI_OnSwitchTurns(AMM_PlayerController* _Player);

#pragma endregion

#pragma region Teams

public:
	/**
	 * Sets up team information in the game mode
	 * such as initial points.
	 * @param _iTeam - team to setup
	 */
	void AddTeam(ETeam _Team);

	/**
	 * Checks if a team has completed all their mice.
	 * @return true if a team has won and the game should end
	 */
	bool HasTeamWon(ETeam _TeamToCheck) const;

protected:
	/** Called when a team has completed in getting all their mice to the other side of the grid */
	void TeamWon(ETeam _Team);

	UFUNCTION(BlueprintImplementableEvent)
		void BI_OnTeamWon(ETeam _Team);

#pragma endregion

#pragma region Score

public:
	/** Get the current score from a given team */
	UFUNCTION(BlueprintPure)
		int GetTeamScore(ETeam _Team) const;

	/**
		* When a mouse reaches the end and should score a point.
		* @param _Mouse - the mouse that completed
		* @return true if the game has reached a win condition
		*/
	void AddScore(ETeam _Team);

#pragma endregion

#pragma region Stalemate

public:
	/**  Described as stalemate, when only 1 mouse exists per team */
	void CheckStalemateMice();

	/**  Find winning team in a stalemate situation */
	ETeam GetWinningStalemateTeam() const;

#pragma endregion

//-------------------------------------------------------

#pragma region Gameloop Variables

public:
	/**  The amount of turns the game will last for when entered stalemate (one mouse per team) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int StalemateTurns = 8;

	/**  The starting number of mice on each team */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int InitialMiceCount = 12;

protected:
	/** The current game play type */
	UPROPERTY(BlueprintReadOnly)
		EGameType CurrentGameType = EGameType::E_NONE;

	/** The current team points, Team ID to number of points */
	UPROPERTY(BlueprintReadOnly)
		TMap<ETeam, int> TeamPoints;

	/** When a stalemate is entered, this value will count up per turn */
	UPROPERTY(BlueprintReadOnly)
		int StalemateCount = -1;

#pragma endregion

#pragma region Player Variables

protected:
	UPROPERTY(BlueprintReadOnly)
		TArray<AMM_PlayerController*> AllPlayers;

	/** The current player whos turn it is */
	UPROPERTY(BlueprintReadOnly)
		AMM_PlayerController* CurrentPlayer;

	/* Stored local player to switch controller*/
	UPROPERTY()
		ULocalPlayer* FirstLocalPlayer;

	/**  Store the second player controller for cleanup */
	UPROPERTY(BlueprintReadOnly)
		APlayerController* SecondLocalPlayerController;

#pragma endregion

#pragma region Grid Variables

public:
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<AMM_GridManager> GridManagerClass;

protected:
	/** The main manager for the grid elements and systems */
	UPROPERTY(BlueprintReadOnly)
		AMM_GridManager* GridManager;

	/** The grid size if no AMM_WorldGrid exists in the level */
	UPROPERTY(EditDefaultsOnly)
		FIntVector2D DefaultGridSize = FIntVector2D(19, 13);

#pragma endregion

};

