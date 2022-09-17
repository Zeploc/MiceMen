// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MM_PlayerController.generated.h"


class AMM_GameViewPawn;
class AMM_GameMode;

/**
 * The main player controller for a playable user, as well as an AI player
 */
UCLASS()
class MICEMEN_API AMM_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMM_PlayerController();

	virtual void OnPossess(APawn* _Pawn) override;

	void SetupPlayer(int _Team);

	/** Changes player to AI */
	virtual void SetAsAI();
	/** Switched player back to human player */
	virtual void ClearAI();

	/** Turn has begun, start interaction */
	virtual void BeginTurn();
	/** Turn has ended */
	virtual void TurnEnded();

	UFUNCTION(BlueprintPure)
		int GetCurrentTeam() { return CurrentTeam; }

	UFUNCTION(BlueprintPure)
		bool IsAI() { return bIsAI; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

protected:
	UPROPERTY(BlueprintReadOnly)
		AMM_GameViewPawn* MMPawn;

	UPROPERTY(BlueprintReadOnly)
		AMM_GameMode* MMGameMode;

	/** The team this player is on */
	UPROPERTY(BlueprintReadOnly)
		int CurrentTeam = -1;

	/** Whether this player is an AI player or a human player */
	UPROPERTY(BlueprintReadOnly)
		bool bIsAI = false;
};
