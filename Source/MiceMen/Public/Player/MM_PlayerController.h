// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Base/MM_GameEnums.h"
#include "MM_PlayerController.generated.h"

class AMM_GameViewPawn;
class AMM_GameMode;
class AMM_ColumnControl;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAITurnComplete, AMM_ColumnControl*, ColumnControl);

/**
 * The main player controller for a playable user, as well as an AI player
 */
UCLASS()
class MICEMEN_API AMM_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMM_PlayerController();

#pragma region Core

	virtual void OnPossess(APawn* _Pawn) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#pragma endregion

#pragma region Setup

public:
	virtual void SetupPlayer(ETeam _Team);

	/** Changes player to AI */
	virtual void SetAsAI();

	/** Switched player back to human player */
	virtual void ClearAI();

#pragma endregion

#pragma region Turn

public:
	/** Turn has begun, start interaction */
	virtual void BeginTurn();

	/** If the player is an AI, perform AI turn */
	virtual bool TakeAITurn();

	/** Turn has ended */
	virtual void TurnEnded();

protected:
	/** Move the column a chosen direction on behalf of the AI player*/
	bool PerformColumnAIMovement(AMM_ColumnControl* _Column, int _Direction) const;
	
	/** Perform AI turn by selecting a random column to move */
	bool TakeRandomAITurn() const;

	/** Perform AI turn by looking for the next opening a mouse can go to and move a column towards that */
	bool TakeAdvancedAITurn() const;

#pragma endregion

#pragma region Gameloop

public:
	UFUNCTION(BlueprintPure)
	ETeam GetCurrentTeam() const { return CurrentTeam; }

	UFUNCTION(BlueprintPure)
	bool IsAI() const { return bIsAI; }

#pragma endregion

//-------------------------------------------------------

#pragma region References

protected:
	UPROPERTY(BlueprintReadOnly)
	AMM_GameViewPawn* MMPawn;

	UPROPERTY(BlueprintReadOnly)
	AMM_GameMode* MMGameMode;

#pragma endregion

#pragma region Gameloop Variables

public:
	/** Called once the AI has performed their turn */
	UPROPERTY(BlueprintAssignable)
	FAITurnComplete OnAITurnComplete;

protected:
	/** The team this player is on */
	UPROPERTY(BlueprintReadOnly)
	ETeam CurrentTeam = ETeam::E_NONE;

	/** Whether this player is an AI player or a human player */
	UPROPERTY(BlueprintReadOnly)
	bool bIsAI = false;

#pragma endregion
};
