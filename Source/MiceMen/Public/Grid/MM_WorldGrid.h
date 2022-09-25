// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IntVector2D.h"
#include "MM_WorldGrid.generated.h"

/**
 * The main control for the grid creation in the world.
 * Will take the World Grids transform to spawn the grid along with the grid size.
 */
UCLASS()
class MICEMEN_API AMM_WorldGrid : public AActor
{
	GENERATED_BODY()

public:
	AMM_WorldGrid();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

public:
	/** The amount of elements on the x and y axis for the grid */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FIntVector2D GridSize = FIntVector2D(19, 13);;
};
