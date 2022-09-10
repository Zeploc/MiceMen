// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid/IntVector2D.h"
#include "MM_GridManager.generated.h"

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


	void SetupGrid(FIntVector2D _GridSize);
	void AdjustColumn(int _Column, int _Direction);

	UFUNCTION(BlueprintPure)
		TMap<int, class AMM_ColumnControl*> GetColumnControls() {
		return ColumnControls;
	}
	UFUNCTION(BlueprintPure)
		FIntVector2D GetGridSize() { return GridSize; }

	UFUNCTION(BlueprintCallable)
		void SetDebugVisualGrid(bool _Enabled);
	UFUNCTION(BlueprintCallable)
		void ToggleDebugVisualGrid();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void RebuildGrid();
	void GridCleanUp();
	void PopulateGrid();

	void PlaceBlock(FIntVector2D _NewCoord, class AMM_ColumnControl* NewColumnControl);

	void PopulateMice();

	void ProcessMice();

	TArray<FVector> PathFromCoordToWorld(TArray<FIntVector2D> _CoordPath);

	FTransform GetWorldTransformFromCoord(FIntVector2D _Coords);

	void DebugVisualiseGrid();


public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<class AMM_GridBlock> GridBlockClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<class AMM_Mouse> MouseClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<class AMM_ColumnControl> ColumnControlClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float GridElementWidth = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float GridElementHeight = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int InitialMiceCount = 12;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FIntVector2D GridSize;

	/**
	 * Active list of mice.
	 */
	TArray<class AMM_Mouse*> Mice;

	/**
	 * Active list of mice per team.
	 * The key is the team, the value is the array
	 */
	TMap<int, TArray<class AMM_Mouse*>> TeamMice;

	/**
	 * Interactable controls for each column.
	 */
	TMap<int, class AMM_ColumnControl*> ColumnControls;


	class UMM_GridObject* GridObject;

	int GapSize;
	int TeamSize;

	bool bDebugGridEnabled = false;
};
