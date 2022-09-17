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

/** Gameplay mode to choose how it players */
UENUM(BlueprintType)
enum class EGameType : uint8
{
	E_NONE		UMETA(DisplayName = "None"),

	/** Player versing Player, turn based one screen */
	E_PVP		UMETA(DisplayName = "Player VS Player"),
	/** Player versing AI, the AI will automatically take a their turn */
	E_PVAI		UMETA(DisplayName = "Player VS AI"),
	/**  For visualizing the game, AI's take turns making their moves until the game ends */
	E_AIVAI		UMETA(DisplayName = "AI VS AI"),
	/** No column or turn restrictions, for playing around, testing interactions and movement */
	E_SANDBOX	UMETA(DisplayName = "Sandbox"),
	/** Runs an instant test to look for any problems */
	E_TEST		UMETA(DisplayName = "Test"),

	E_MAX		UMETA(DisplayName = "MAX")
};

/**
 * Control for the main game play, grid systems and players
 */
UCLASS()
class MICEMEN_API AMM_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMM_GameMode();

	UFUNCTION(BlueprintPure)
		AMM_GridManager* GetGridManager();

	/** Starts game with a certain game play type */
	UFUNCTION(BlueprintCallable)
		void BeginGame(EGameType _GameType);
	/** Changes current mode to test and switches players to AI */
	UFUNCTION(BlueprintCallable)
		void SwitchToTest();
	/** Completes the game, and cleans up all grid systems */
	UFUNCTION(BlueprintCallable)
		void EndGame();

	/**
	* Once a player's turn is complete, performs checks
	* Will either complete the game or go to the next player. 
	* @param _Player the player who's turn just ended
	*/
	void PlayerTurnComplete(AMM_PlayerController* _Player);

	/**  Described as stalemate, when only 1 mouse exists per team */
	void CheckStalemateMice();

	/**  Find winning team in a stalemate situation */
	int GetWinningStalemateTeam();

	/** Get the player who's current turn it is */
	UFUNCTION(BlueprintPure)
	AMM_PlayerController* GetCurrentPlayer() { return CurrentPlayer; }

	UFUNCTION(BlueprintPure)
		EGameType GetCurrentGameType() { return CurrentGameType; }

	/** Get the current score from a given team */
	UFUNCTION(BlueprintPure)
		int GetTeamScore(int _Team);

	/**
	 * Sets up team information in the game mode
	 * such as initial points.
	 * @param _iTeam - team to setup
	 */
	void AddTeam(int _iTeam);

	/**
	 * When a mouse reaches the end and should score a point.
	 * @param _Mouse - the mouse that completed
	 * @return true if the game has reached a win condition
	 */
	bool MouseCompleted(AMM_Mouse* _Mouse);


	/**
	 * Checks if a team has completed all their mice.
	 * @return true if a team has one and the game should end
	 */
	bool CheckWinCondition();

	/**  Resets the board and players */
	UFUNCTION(BlueprintCallable)
	void RestartGame();

	/** Clean up grid and restore to starting state */
	void CleanupGame();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type _EndPlayReason) override;

	/** Called when the game is ready and game play mode can be chosen */
	void GameReady();

	/** Creates grid and basic setup, uses AMM_WorldGrid to position the grid*/
	bool SetupGridManager();

	/** When a new player joins, will store and check if the game is ready to start */
	virtual void PostLogin(APlayerController* _NewPlayer) override;

	/** Will change turns to a different player */
	void SwitchTurns(AMM_PlayerController* _Player);

	/** Called when a team has completed in getting all their mice to the other side of the grid */
	void TeamWon(int _iTeam);

	UFUNCTION(BlueprintImplementableEvent)
		void BI_OnGameReady();
	UFUNCTION(BlueprintImplementableEvent)
		void BI_OnGameBegun();
	UFUNCTION(BlueprintImplementableEvent)
		void BI_OnGameEnded();
	UFUNCTION(BlueprintImplementableEvent)
		void BI_OnGameRestarted();

	UFUNCTION(BlueprintImplementableEvent)
		void BI_OnSwitchTurns(AMM_PlayerController* _Player);

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
	/** The main manager for the grid elements and systems */
	UPROPERTY(BlueprintReadOnly)
	class AMM_GridManager* GridManager;

	/** The grid size if no AMM_WorldGrid exists in the level */
	UPROPERTY(EditDefaultsOnly)
		FIntVector2D DefaultGridSize = FIntVector2D(19, 13);

	UPROPERTY(BlueprintReadOnly)
		TArray<AMM_PlayerController*> AllPlayers;

	/** The current player whos turn it is */
	UPROPERTY(BlueprintReadOnly)
		AMM_PlayerController* CurrentPlayer;

	/** The current team points, Team ID to number of points */
	UPROPERTY(BlueprintReadOnly)
	TMap<int, int> TeamPoints;

	/* Stored local player to switch controller*/
	UPROPERTY()
	ULocalPlayer* FirstLocalPlayer;

	/** When a stalemate is entered, this value will count up per turn */
	UPROPERTY(BlueprintReadOnly)
		int StalemateCount = -1;

	/** The current game play type */
	UPROPERTY(BlueprintReadOnly)
	EGameType CurrentGameType = EGameType::E_NONE;

	/**  Store the second player controller for cleanup */
	UPROPERTY(BlueprintReadOnly)
	APlayerController* SecondLocalPlayerController;
};

