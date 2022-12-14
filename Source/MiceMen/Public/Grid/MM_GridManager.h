// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid/IntVector2D.h"
#include "Base/MM_GameEnums.h"
#include "Base/MM_GameMode.h"
#include "Base/MM_GridEnums.h"
#include "Player/MM_PlayerController.h"
#include "MM_GridManager.generated.h"

class AMM_ColumnControl;
class AMM_Mouse;
class AMM_GridElement;
class AMM_GridBlock;
class UMM_GridObject;
class AMM_GameMode;

/**
 * Handles the main grid operations, such as setup and moving blocks
 */
UCLASS()
class MICEMEN_API AMM_GridManager : public AActor
{
	GENERATED_BODY()

public:
	AMM_GridManager();

#pragma region Core

public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

#pragma endregion

#pragma region Stalemate

public:
	/** A check for if only one mouse per team exists */
	UFUNCTION(BlueprintPure)
	bool IsStalemate() const;

	/** When a stalemate win condition occurs, get the further ahead mouse as the winning team */
	UFUNCTION(BlueprintPure)
	ETeam GetWinningStalemateTeam(int& DistanceWonBy) const;

	/** Check if mice have taken up opposite columns resulting in no way to get passed, so no moves to finish the game */
	UFUNCTION(BlueprintPure)
	bool CheckNoValidMoves();

#pragma endregion

#pragma region Grid Setup

public:
	/** Stores initial information */
	void SetupGridVariables(const FIntVector2D& InGridSize, AMM_GameMode* InMMGameMode);

	/** Sets up grid object and initial sizes */
	void CreateGrid();

	/** Cleans up grid and recreates it */
	void RebuildGrid(const int InitialMiceCount);

	UFUNCTION(BlueprintPure)
	FIntVector2D GetGridSize() const { return GridSize; }

protected:
	/** Handles grid cleanup, removing and clearing objects */
	void GridCleanUp();

	/** Populates initial blocks and empty spaces */
	void PopulateGrid();

	/** Center blocks with a set pattern either side of the center gap so mice don't initially cross over */
	bool IsCoordInCenterGroup(const FIntVector2D& NewCoord) const;

	/** Decides what element to place, either block or empty */
	void PlaceGridElement(const FIntVector2D& NewCoord, AMM_ColumnControl* ColumnControl);

	/** Places initial mice for each team, based on MicePerTeam */
	void PopulateTeams(int MicePerTeam);

#pragma endregion

#pragma region Mouse Processing

public:
	/** Starts processing all mice, starting with the current players team */
	void BeginProcessMice();

	/** Removes a mouse from the active list and teams */
	void RemoveMouse(AMM_Mouse* Mouse);

	/** Moves a mouse to a new location, clearing the old location */
	void SetMousePosition(AMM_Mouse* Mouse, const FIntVector2D& NewCoord);

	/** Attempts to move a grid element to a new coordinate, returns true if successful */
	bool MoveGridElement(AMM_GridElement* GridElement, const FIntVector2D& NewCoord) const;

	TMap<ETeam, TArray<AMM_Mouse*>> GetMiceTeams() { return MiceTeams; }

protected:
	/** Find all the mice to process and store them in order of position and team */
	void StoreOrderedMiceToProcess();

	/* Add mouse to array based on positioning so lower forward mice go first */
	void InsertMouseByOrder(AMM_Mouse* TeamMouse, TArray<AMM_Mouse*>& CurrentTeamMiceToProcess);

	/** Ends the players turn when there are no more mice to process. */
	void HandleMiceComplete();

	/** Handles moving mouse and updating grid */
	void ProcessMouse(AMM_Mouse* Mouse);

	/** Checks mouse is valid to process, will continue to next mouse if it is not */
	bool CheckMouse(AMM_Mouse* Mouse);

	/** Called once a mouse has been processed */
	UFUNCTION()
	void HandleCompletedMouseMovement(AMM_Mouse* Mouse);

	/** Clears Mouse from processing and unbinds delegates. */
	void CleanupProcessedMouse(AMM_Mouse* Mouse);

#pragma endregion

#pragma region Column Processing

public:
	/** Moves the specified column in a direction, either up or down (1 or -1) */
	void AdjustColumn(int Column, EDirection Direction);

	/** Attempts to adjust the column in the grid object, updating the grid elements */
	bool AdjustColumnInGridObject(int Column, EDirection Direction, AMM_GridElement*& LastElement) const;

	UFUNCTION(BlueprintPure)
	bool IsTeamInColumn(int Column, ETeam Team) const;

	UFUNCTION(BlueprintPure)
	TArray<int> GetTeamColumns(ETeam Team) const;

	UFUNCTION(BlueprintPure)
	TMap<int, AMM_ColumnControl*> GetColumnControls() const { return ColumnControls; }

protected:
	/** Removes mouse from specified column, updating team/mouse variables */
	void RemoveMouseFromColumn(int Column, AMM_Mouse* Mouse);

	/** Adds mouse to a specified column, updating team/mouse variables */
	void AddMouseToColumn(int Column, AMM_Mouse* Mouse);

#pragma endregion

#pragma region Helpers

public:
	/** Helper for converting coordinates path to world space */
	UFUNCTION(BlueprintPure)
	TArray<FVector> PathCoordToWorld(const TArray<FIntVector2D>& CoordPath) const;

	/** Helper for converting coordinates to world space */
	UFUNCTION(BlueprintPure)
	FTransform CoordToWorldTransform(const FIntVector2D& Coords) const;

	/** Direction along the grid the team goes */
	UFUNCTION(BlueprintPure)
	static EDirection GetDirectionFromTeam(ETeam Team);

	/** Will check one slot in a given direction, and return true if its free, setting CurrentPosition */
	UFUNCTION(BlueprintPure)
	bool FindFreeSlotInDirection(FIntVector2D& CurrentPosition, const FIntVector2D& Direction) const;

	/** Will check for the lowest possible free slot below without passing through a taken element, and return true if its free, setting CurrentPosition */
	UFUNCTION(BlueprintPure)
	bool FindFreeSlotBelow(FIntVector2D& CurrentPosition) const;

	/** Will check one slot ahead horizontally, based on the given direction, and return true if its free, setting CurrentPosition */
	UFUNCTION(BlueprintPure)
	bool FindFreeSlotAhead(FIntVector2D& CurrentPosition, EDirection Direction) const;

#pragma endregion

#pragma region Debug

public:
	/** Shows a visual for the all the grid elements stored in the grid object */
	UFUNCTION(BlueprintCallable)
	void SetDebugVisualGrid(bool bEnabled);

	/** Toggles the grid visual */
	UFUNCTION(BlueprintCallable)
	void ToggleDebugVisualGrid();

protected:
	/** Called on tick when the debug grid is enabled, to draw visuals representing grid elements */
	void DisplayDebugGrid() const;

	/** Checks all mice are included in the mice that are set to be processed */
	bool DebugCheckAllMiceProcessed() const;

#pragma endregion

//-------------------------------------------------------

#pragma region Subclass Variables

public:
	/** Configuration of classes to spawn */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AMM_GridBlock> GridBlockClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AMM_Mouse> MouseClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AMM_ColumnControl> ColumnControlClass;

#pragma endregion

#pragma region Grid Variables

public:
	/** World space width for one grid element */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float GridElementWidth = 100.0f;

	/** World space height for one grid element */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float GridElementHeight = 100.0f;

protected:
	/** The grid size set by the game mode used to populate the grid */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FIntVector2D GridSize = FIntVector2D(19, 13);

	/** The current size of the gap between teams along the x axis */
	UPROPERTY(BlueprintReadOnly)
	int GapSize;

	/** The amount of columns for one team when initially spawning the mice */
	UPROPERTY(BlueprintReadOnly)
	int TeamSize;

	/** How sparse the block placement should be */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BlockSparseness = 0.4f;

	/** The main control object for grid elements */
	UPROPERTY()
	UMM_GridObject* GridObject;

#pragma endregion

#pragma region Mice Variables

protected:
	/**
	* Active list of mice.
	*/
	UPROPERTY(BlueprintReadOnly)
	TArray<AMM_Mouse*> Mice;

	/**
	* List of mice that have reached their goal.
	*/
	UPROPERTY(BlueprintReadOnly)
	TArray<AMM_Mouse*> CompletedMice;

	/** Current Mice to process movement from a column change. */
	TArray<AMM_Mouse*> MiceToProcessMovement;

	/**
	* Active list of mice per team.
	* @key the team
	* @value the array of mice in the team
	*/
	TMap<ETeam, TArray<AMM_Mouse*>> MiceTeams;

	/* Used to keep track of the moved mice in process, only once none have moved will it complete */
	int CurrentProcessedMovedMiceCount = 0;

#pragma endregion

#pragma region Column Variables

protected:
	/**
	* Active list of all the mice per column.
	* @key the column by x axis
	* @value the array of mice in that column
	*/
	TMap<int, TArray<AMM_Mouse*>> MiceColumns;

	/**
	* Active list of columns for what teams are occupying
	* @key the column by x axis
	* @value the array of teams on that column
	*/
	TMap<int, TArray<ETeam>> OccupiedTeamsPerColumn;

	/**
	* Interactable controls for each column.
	*/
	TMap<int, AMM_ColumnControl*> ColumnControls;

	/** The last moved column to stop repeat moves */
	UPROPERTY(BlueprintReadOnly)
	int LastMovedColumn = -1;

#pragma endregion

#pragma region References

protected:
	UPROPERTY(BlueprintReadOnly)
	AMM_GameMode* MMGameMode;

	UPROPERTY()
	AMM_PlayerController* CurrentPlayerProcessing;

#pragma endregion

#pragma region Debug Variables

	/** Whether the debug grid is currently visualized */
	UPROPERTY(BlueprintReadOnly)
	bool bDisplayDebugGrid = false;

#pragma endregion
};
