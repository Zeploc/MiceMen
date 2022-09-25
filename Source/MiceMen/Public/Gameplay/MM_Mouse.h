// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "Grid/MM_GridElement.h"
#include "Base/MM_GameEnums.h"
#include "MM_Mouse.generated.h"

/**
* The delegate for when a mouse has completed their movement to the final position
* @OwningMouse the mouse that has completed the movement
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMouseMovementEndDelegate, class AMM_Mouse*, OwningMouse);

/**
 * The mouse grid element which auto moves, scoring a point when reaching the end
 */
UCLASS()
class MICEMEN_API AMM_Mouse : public AMM_GridElement
{
	GENERATED_BODY()

public:
	AMM_Mouse();

#pragma region Core

public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

#pragma endregion

#pragma region Team

public:
	/** Stores team and coordinates to start at */
	virtual void SetupMouse(const ETeam InTeam, FIntVector2D& FinalGridCoordinates);

	UFUNCTION(BlueprintPure)
	ETeam GetTeam() const { return CurrentTeam; };

#pragma endregion

#pragma region Movement

public:
	/** Attempt to move the mouse, return true is movement successful */
	bool AttemptPerformMovement();

	/** Gets a valid path for this mouse, horizontal direction based on the team to move towards */
	UFUNCTION(BlueprintCallable)
	virtual TArray<FIntVector2D> GetMovementPath() const;

protected:
	/** Move mouse to next valid position, returns true if mouse moved */
	virtual bool BeginMove(FIntVector2D& NewPosition);

	/**
	 * Override for visual movement,
	 * call MouseMovementEndDelegate once complete, or game will halt.
	 *
	 * @param Path - The movements to make
	 */
	UFUNCTION(BlueprintNativeEvent)
	void BN_StartMovement(const TArray<FVector>& Path);

	/** Update the grid based on the new position */
	void ProcessUpdatedPosition(const FIntVector2D& NewPosition);

#pragma endregion

#pragma region Goal

public:
	UFUNCTION(BlueprintPure)
	bool HasReachedEnd() const { return bGoalReached; };

protected:
	/** When a mouse has reached the end of the grid for their team */
	virtual void GoalReached();

	UFUNCTION(BlueprintImplementableEvent)
	void BI_OnGoalReached();

#pragma endregion

#pragma region Debug

public:
	/** Displays a path in world space using colored boxes, increasing in size down the path */
	UFUNCTION(BlueprintCallable)
	void DisplayDebugPath(const TArray<FIntVector2D>& ValidPath) const;

#pragma endregion

//-------------------------------------------------------

#pragma region Team Variables

protected:
	/** The mouse's team */
	UPROPERTY(BlueprintReadOnly)
	ETeam CurrentTeam = ETeam::E_NONE;

#pragma endregion

#pragma region Movement Variables

public:
	/** Executed when the movement has been made, to continue the next event */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FMouseMovementEndDelegate MovementEndDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDisplayDebugPath = false;

#pragma endregion

#pragma region Goal Variables

protected:
	/** Whether the mouse has reached the other side of the grid */
	UPROPERTY(BlueprintReadOnly)
	bool bGoalReached = false;

#pragma endregion
};
