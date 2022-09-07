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
	void PopulateGrid();


public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FIntVector GridSize;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class AMM_GridBlock> GridBlockClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float GridElementWidth = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float GridElementHeight = 100.0f;

protected:
	/**
	 * Note: If optimisation was a focus, would look into using a one dimensional array, with look up and helper utils
	 */
	TArray<TArray<class UMM_GridInfo*>> Grid;

};
