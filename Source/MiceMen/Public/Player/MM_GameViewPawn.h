// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MM_GameViewPawn.generated.h"

class UInputComponent;
class USceneComponent;
class UCineCameraComponent;
class AMM_ColumnControl;
class AMM_GridManager;
class AMM_PlayerController;
class AMM_GameMode;

/*
* The main pawn for viewing and interacting with the grid
*/
UCLASS()
class MICEMEN_API AMM_GameViewPawn : public APawn
{
	GENERATED_BODY()

public:
	AMM_GameViewPawn();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCineCameraComponent* GameCamera;

#pragma region Core

public:
	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

	/** Called to bind functionality to input */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Called when the pawn is possessed by a controller, stores MM controller */
	virtual void PossessedBy(AController* NewController) override;

#pragma endregion

#pragma region Turns

public:
	/** Called on the beginning of the players turn, stores available columns to interact with */
	virtual void BeginTurn();	

	UFUNCTION(BlueprintPure)
	bool IsTurnActive() const { return bTurnActive; };

protected:
	/** Called when the turn ends, cleans up columns information */
	virtual void TurnEnded();

	UFUNCTION()
	void AITurnComplete(AMM_ColumnControl* ColumnControl);

#pragma endregion

#pragma region Columns

public:
	/** Stores a column as grabbable to interact with during the player's current turn */
	virtual void AddColumnAsGrabbable(int Column);

	/** Gets the current interactable columns for this player */
	UFUNCTION(BlueprintPure)
	virtual TArray<AMM_ColumnControl*> GetCurrentColumnControls() const { return CurrentColumnControls; };

protected:
	/** Updates column interaction count, and last interacted column */
	void UpdateColumnInteractionCount();

	/** Called once a column has been moved, and passed in true if the column had changed, completing the turn */
	virtual void ProcessMovedColumn(bool bTurnComplete);

#pragma endregion

#pragma region Interaction

protected:
	/** Handles the player interaction for initial grabbing of a column */
	virtual void BeginGrab();

	/** Called from tick, updates the columns projected position based on the cursor position  */
	virtual void HandleGrab();

	/** Released the current column */
	virtual void EndGrab();
	
#pragma endregion

#pragma region Getters

protected:
	UFUNCTION(BlueprintPure)
	AMM_GridManager* GetGridManager();

	UFUNCTION(BlueprintPure)
	AMM_GameMode* GetMMGamemode();

#pragma endregion

//-------------------------------------------------------

#pragma region Turn Variables

protected:
	/** Whether this player's turn is currently active */
	UPROPERTY(BlueprintReadOnly)
	bool bTurnActive = false;

#pragma endregion

#pragma region Interaction Variables

public:
	/** The distance the player can interact when grabbing */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float InteractTraceDistance = 100000.0f;

protected:
	/** The offset from the initial grab of the current column */
	FVector HitColumnOffset;

#pragma endregion

#pragma region Column Variables

public:
	/**
	* Maximum times the same column can be moved by the player.
	* Superseded if all mouse on that same column
	* Stored in pawn to have the ability to change it per player
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int SameColumnMax = 6;

protected:
	/** The column currently grabbed by the player */
	UPROPERTY(BlueprintReadOnly)
	AMM_ColumnControl* CurrentColumn;

	/** The available columns for this player to interact with */
	UPROPERTY(BlueprintReadOnly)
	TArray<AMM_ColumnControl*> CurrentColumnControls;

	/** Linked to the current column on release, for when the column slots into place */
	FDelegateHandle CurrentColumnDelegateHandle;

	/** The last column that was moved by this player */
	UPROPERTY(BlueprintReadOnly)
	int LastMovedColumn = -1;

	/** The amount of times the LastMovedColumn has been moved in a row */
	UPROPERTY(BlueprintReadOnly)
	int SameMovedColumnCount = 0;

#pragma endregion

#pragma region References Variables

protected:
	UPROPERTY(BlueprintReadOnly)
	AMM_PlayerController* MMPlayerController;

	UPROPERTY()
	AMM_GameMode* MMGameMode;

	UPROPERTY()
	AMM_GridManager* GridManager;

#pragma endregion

};
