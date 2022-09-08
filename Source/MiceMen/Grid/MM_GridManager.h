// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void SetupGrid();
	void PopulateGridBlocks();
	void PopulateMice();

	int CoordToIndex(int _X, int _Y);

	FTransform GetWorldTransformFromCoord(int _X, int _Y);
	FIntVector GetRandomGridCoord();
	FIntVector GetRandomGridCoordInColumnRange(int _MinX, int _MaxX);
	FIntVector GetRandomGridCoordInRange(int _MinX, int _MaxX, int _MinY, int _MaxY);


public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FIntVector GridSize;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class AMM_GridBlock> GridBlockClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float GridElementWidth = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float GridElementHeight = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int InitialMiceCount = 12;

protected:
	/**
	 * One dimensional array for two dimensional grid
	 */
	TArray<class UMM_GridInfo*> Grid;

	/**
	 * Coord to index position.
	 * Used as lookup, updated when a change in the grid occurs
	 * Could store key as array index, but storing as coords
	 * removes a need to run an operation to calculate from index to coord
	 */
	TMap<FIntVector, int> FreeSlots;

	/**
	 * Generated from FreeSlots to have array to remove keys generation.
	 * This is to remove the GenerateKeys() from the map, created an extra iteration
	 * This is based on CPU being prioritised over memory
	 */
	TArray<FIntVector> FreeSlotKeys;
};
