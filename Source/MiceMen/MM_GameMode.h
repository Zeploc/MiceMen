// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Grid/IntVector2D.h"
#include "MM_GameMode.generated.h"

/**
 * Control for the main gameplay, grid systems and players
 */
UCLASS()
class MICEMEN_API AMM_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMM_GameMode();

	UFUNCTION(BlueprintPure)
		class AMM_GridManager* GetGridManager();

protected:
	virtual void BeginPlay() override;

	/** Creates grid and basic setup */
	bool SetupGridManager();


public:
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class AMM_GridManager> GridManagerClass;

protected:
	UPROPERTY(BlueprintReadOnly)
	class AMM_GridManager* GridManager;

	UPROPERTY(EditDefaultsOnly)
		FIntVector2D DefaultGridSize = FIntVector2D(19, 13);
};

