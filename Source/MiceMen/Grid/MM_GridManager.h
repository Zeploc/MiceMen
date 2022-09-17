// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid/IntVector2D.h"
#include "MM_GridManager.generated.h"

class AMM_ColumnControl;
class AMM_Mouse;
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

#pragma region Game Loop

public:	
	// Sets default values for this actor's properties
	AMM_GridManager();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** A check for if only one mouse per team exists */
	bool IsStalemate() const;

	/**  When a stalemate win condition occurs, get the further ahead mouse as the winning team */
	int GetWinningStalemateTeam() const;


protected:
	// Begin and End play events
	virtual void BeginPlay() override;
	void EndPlay(EEndPlayReason::Type _EndPlayReason);

#pragma endregion

#pragma region Grid Setup and Cleanup

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


	/** Called once a mouse has been processed */
	UFUNCTION()
	void OnMouseProcessed(AMM_Mouse* _Mouse);

	/** Handles moving mouse and updating grid */
	void ProcessMouse(AMM_Mouse* _Mouse);

	/** Move mouse to next valid position, returns true if mouse moved */
	bool MoveMouse(AMM_Mouse* _NextMouse, FIntVector2D& _FinalPosition);

	/** When a mouse has reached the end of the grid for their team */
	void MouseCompleted(AMM_Mouse* _NextMouse, int iTeam);
		

#pragma endregion

#pragma region Column Processing

public:
	/** Moves the specified column in a direction, either up or down (1 or -1) */
	void AdjustColumn(int _Column, int _Direction);

	UFUNCTION(BlueprintPure)
		bool IsTeamInColumn(int _Column, int _Team);
	UFUNCTION(BlueprintPure)
		TArray<int> GetTeamColumns(int _Team);
	UFUNCTION(BlueprintPure)
		TMap<int, AMM_ColumnControl*> GetColumnControls() {
		return ColumnControls;
	}

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

	UFUNCTION(BlueprintCallable)
		void SetDebugVisualGrid(bool _Enabled);
	UFUNCTION(BlueprintCallable)
		void ToggleDebugVisualGrid();

	bool DebugCheckAllMiceProcessed(int iTeam, const TArray<AMM_Mouse*>& CurrentTeamMiceToProcess) const;

	UFUNCTION(BlueprintCallable)
		void EnableTestMode();
	UFUNCTION(BlueprintCallable)
		void StartTest();

protected:

	void DebugVisualiseGrid();

	void DebugPath(TArray<FIntVector2D> ValidPath);

#pragma endregion


public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<AMM_GridBlock> GridBlockClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<AMM_Mouse> MouseClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<AMM_ColumnControl> ColumnControlClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float GridElementWidth = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float GridElementHeight = 100.0f;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FIntVector2D GridSize;

	/**
	 * Active list of mice.
	 */
	TArray<AMM_Mouse*> Mice;

	/**
	 * Active list of mice per team.
	 * The key is the team, the value is the array
	 */
	TMap<int, TArray<AMM_Mouse*>> TeamMice;

	/**
	 * Active list of all the mice per column.
	 * The key is the column, the value is the array of mice
	 */
	TMap<int, TArray<AMM_Mouse*>> MouseColumns;

	/**
	 * Active list of teams per column.
	 * The key is the column, the value is the array of teams on that column
	 */
	TMap<int, TArray<int>> AvailableColumnTeams;

	/**
	 * Interactable controls for each column.
	 */
	TMap<int, AMM_ColumnControl*> ColumnControls;

	UPROPERTY(BlueprintReadOnly)
		int LastMovedColumn = -1;

	UPROPERTY()
	UMM_GridObject* GridObject;

	UPROPERTY(BlueprintReadOnly)
	AMM_GameMode* MMGameMode;

	/**
	 * Current Mice to process movement.
	 */
	TArray<AMM_Mouse*> MiceToProcessMovement;

	int GapSize;
	int TeamSize;

	bool bDebugGridEnabled = false;
	bool bDebugTest = false;
};
