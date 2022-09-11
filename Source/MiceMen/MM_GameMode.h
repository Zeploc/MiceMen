// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Grid/IntVector2D.h"
#include "MM_GameMode.generated.h"

class APlayerController;
class AMM_PlayerController;
class AMM_GridManager;

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

	void PlayerTurnComplete(AMM_PlayerController* _Player);

	AMM_PlayerController* GetCurrentPlayer() { return CurrentPlayer; }

protected:
	virtual void BeginPlay() override;

	void BeginGame();

	/** Creates grid and basic setup */
	bool SetupGridManager();

	virtual void PostLogin(APlayerController* _NewPlayer) override;

	void SwitchTurns(AMM_PlayerController* _Player);

	UFUNCTION(BlueprintImplementableEvent)
	void BI_OnSwitchTurns(AMM_PlayerController* _Player);


public:
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<AMM_GridManager> GridManagerClass;

protected:
	UPROPERTY(BlueprintReadOnly)
	class AMM_GridManager* GridManager;

	UPROPERTY(EditDefaultsOnly)
		FIntVector2D DefaultGridSize = FIntVector2D(19, 13);

	UPROPERTY(BlueprintReadOnly)
		TArray<AMM_PlayerController*> AllPlayers;

	UPROPERTY(BlueprintReadOnly)
		AMM_PlayerController* CurrentPlayer;
};

