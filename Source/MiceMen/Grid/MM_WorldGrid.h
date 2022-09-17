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
	// Sets default values for this actor's properties
	AMM_WorldGrid();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	/** The amount of elements on the x and y axis for the grid */
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
		FIntVector2D GridSize;

protected:
};
