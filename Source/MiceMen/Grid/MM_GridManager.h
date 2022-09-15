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
	
public:	
	// Sets default values for this actor's properties
	AMM_GridManager();

	// Called every frame
	virtual void Tick(float DeltaTime) override;


	void SetupGrid(FIntVector2D _GridSize, AMM_GameMode* _MMGameMode);
	void RebuildGrid(int _InitialMiceCount);
	void AdjustColumn(int _Column, int _Direction);

	UFUNCTION(BlueprintPure)
		TMap<int, AMM_ColumnControl*> GetColumnControls() {
		return ColumnControls;
	}
	UFUNCTION(BlueprintPure)
		FIntVector2D GetGridSize() { return GridSize; }

	UFUNCTION(BlueprintCallable)
		void SetDebugVisualGrid(bool _Enabled);
	UFUNCTION(BlueprintCallable)
		void ToggleDebugVisualGrid();

	UFUNCTION(BlueprintPure)
		bool IsTeamInColumn(int _Column, int _Team);
	UFUNCTION(BlueprintPure)
		TArray<int> GetTeamColumns(int _Team);

	/** A check for if only one mouse per team exists */
	bool IsStalemate() const;

	/**  When a stalemate win condition occurs, get the further ahead mouse as the winning team */
	int GetWinningStalemateTeam() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void EndPlay(EEndPlayReason::Type _EndPlayReason);

	void GridCleanUp();
	void PopulateGrid();

	void PlaceBlock(FIntVector2D _NewCoord, AMM_ColumnControl* NewColumnControl);


	void PopulateMice(int _MicePerTeam);

	void BeginProcessMice();

	UFUNCTION()
	void OnMouseProcessed(AMM_Mouse* _Mouse);

	void ProcessMouse(AMM_Mouse* _NextMouse);

	void MouseCompleted(AMM_Mouse* _NextMouse, int iTeam);

	void DebugPath(TArray<FIntVector2D> ValidPath);

	void RemoveMouseFromColumn(int _Column, AMM_Mouse* _Mouse);
	void AddMouseToColumn(int _Column, AMM_Mouse* _Mouse);

	TArray<FVector> PathFromCoordToWorld(TArray<FIntVector2D> _CoordPath) const;

	FTransform GetWorldTransformFromCoord(FIntVector2D _Coords) const;

	void DebugVisualiseGrid();


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
};
