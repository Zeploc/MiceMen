// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
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
	
};

