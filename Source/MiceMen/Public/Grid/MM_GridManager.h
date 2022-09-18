// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid/IntVector2D.h"
#include "Base/MM_Enums.h"
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
	// Sets default values for this actor's properties
	AMM_GridManager();

#pragma region Game Loop

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** A check for if only one mouse per team exists */
	bool IsStalemate() const;

	/** When a stalemate win condition occurs, get the further ahead mouse as the winning team */
	ETeam GetWinningStalemateTeam() const;

protected:
	// Begin and End play events
	virtual void BeginPlay() override;
	void EndPlay(EEndPlayReason::Type _EndPlayReason);

#pragma endregion

#pragma region Grid Setup

public:
	/** Stores initial information */
	void SetupGrid(FIntVector2D _GridSize, AMM_GameMode* _MMGameMode);

	/** Sets up grid object */
	void CreateGrid();

	/** Cleans up grid and recreates it */
	void RebuildGrid(int _InitialMiceCount);

	UFUNCTION(BlueprintPure)
		FIntVector2D GetGridSize() { return GridSize; }

protected:

	/** Handles grid cleanup, removing and clearing objects */
	void GridCleanUp();

	/** Populates initial blocks and empty spaces */
	void PopulateGrid();

	/** Decides what element to place, either block or empty */
	void PopulateGridElement(FIntVector2D _NewCoord, AMM_ColumnControl* NewColumnControl);

	/** Places initial mice for each team, based on _MicePerTeam */
	void PopulateMice(int _MicePerTeam);

#pragma endregion

#pragma region Mouse Processing

protected:

	/** Starts processing all mice, starting with the current players team */
	void BeginProcessMice();

	/** Find all the mice to process and store them in order of position and team */
	void StoreOrderedMiceToProcess();

	/** Ends the players turn when there are no more mice to process. */
	void ProcessMiceComplete();

	/** Handles moving mouse and updating grid */
	void ProcessMouse(AMM_Mouse* _Mouse);

	/** Move mouse to next valid position, returns true if mouse moved */
	bool MoveMouse(AMM_Mouse* _NextMouse, FIntVector2D& _FinalPosition);

	/** Called once a mouse has been processed */
	UFUNCTION()
		void ProcessCompletedMouseMovement(AMM_Mouse* _Mouse);

	/**
	* Clears from processing and unbinds delegates.
	* @param _Mouse to cleanup
	*/
	void CleanupProcessedMouse(AMM_Mouse* _Mouse);

	/** When a mouse has reached the end of the grid for their team */
	void MouseGoalReached(AMM_Mouse* _NextMouse, ETeam _iTeam);

#pragma endregion

#pragma region Column Processing

public:
	/** Moves the specified column in a direction, either up or down (1 or -1) */
	void AdjustColumn(int _Column, int _Direction);


	UFUNCTION(BlueprintPure)
		bool IsTeamInColumn(int _Column, ETeam _Team);

	UFUNCTION(BlueprintPure)
		TArray<int> GetTeamColumns(ETeam _Team);

	UFUNCTION(BlueprintPure)
		TMap<int, AMM_ColumnControl*> GetColumnControls() {	return ColumnControls; }

protected:
	/** Removes mouse from specified column, updating team/mouse variables */
	void RemoveMouseFromColumn(int _Column, AMM_Mouse* _Mouse);

	/** Adds mouse to a specified column, updating team/mouse variables */
	void AddMouseToColumn(int _Column, AMM_Mouse* _Mouse);

#pragma endregion

#pragma region Helpers

	/** Helpers for converting coordinates to world and back */
	TArray<FVector> PathFromCoordToWorld(TArray<FIntVector2D> _CoordPath) const;

	FTransform GetWorldTransformFromCoord(FIntVector2D _Coords) const;

#pragma endregion

#pragma region Debug

public:
	
	/** Shows a visual for the all the grid elements stored in the grid object */
	UFUNCTION(BlueprintCallable)
		void SetDebugVisualGrid(bool _bEnabled);

	/** Toggles the grid visual */
	UFUNCTION(BlueprintCallable)
		void ToggleDebugVisualGrid();

	/** Checks all mice are included in the mice that are set to be processed */
	bool DebugCheckAllMiceProcessed() const;


protected:
	/** Called on tick when the debug grid is enabled, to draw visuals representing grid elements */
	void DisplayDebugVisualiseGrid();

	/** Displays a path in world space using colored boxes, increasing in size down the path */
	void DebugPath(TArray<FIntVector2D> ValidPath);

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
		FIntVector2D GridSize;

	/** The current size of the gap between teams along the x axis */
	int GapSize;

	/** The amount of columns for one team when initially spawning the mice */
	int TeamSize;

	/** The main control object for grid elements */
	UPROPERTY()
		UMM_GridObject* GridObject;

#pragma endregion

#pragma region Mice Variables

protected:
	/**
	 * Active list of mice.
	 */
	UPROPERTY()
		TArray<AMM_Mouse*> Mice;

	/** Current Mice to process movement from a column change. */
	TArray<AMM_Mouse*> MiceToProcessMovement;

	/**
	 * Active list of mice per team.
	 * @key the team
	 * @value the array of mice in the team
	 */
	TMap<ETeam, TArray<AMM_Mouse*>> MiceTeams;


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

#pragma endregion

#pragma region Debug Variables

	/** Whether the debug grid is currently visualized */
	bool bDebugGridEnabled = false;

#pragma endregion

};
