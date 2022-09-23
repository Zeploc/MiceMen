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
	// Sets default values for this actor's properties
	AMM_Mouse();

#pragma region Core

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#pragma endregion

#pragma region Team

public:
	/** Stores team */
	void SetupMouse(ETeam _Team, FIntVector2D& _FinalGridCoordinates);

	ETeam GetTeam() const { return CurrentTeam; };

#pragma endregion

#pragma region Movement

public:
	/**
	 * Override for visual movement,
	 * call MouseMovementEndDelegate once complete, or game will halt.
	 *
	 * @param _Path - The movements to make
	 */
	UFUNCTION(BlueprintNativeEvent)
	void BN_StartMovement(const TArray<FVector>& _Path);

	/** Attempt to move the mouse, return true is movement successful */
	bool AttemptPerformMovement();

	/** Gets a valid path for this mouse, horizontal direction based on the team to move towards */
	TArray<FIntVector2D> GetMovementPath() const;

protected:
	/** Move mouse to next valid position, returns true if mouse moved */
	bool BeginMove(FIntVector2D& _NewPosition);

	/** Update the grid based on the new position */
	void ProcessUpdatedPosition(const FIntVector2D& _NewPosition);


#pragma endregion

#pragma region Goal

public:
	bool HasReachedEnd() const { return bGoalReached; };

protected:
	/** When a mouse has reached the end of the grid for their team */
	void GoalReached();

	UFUNCTION(BlueprintImplementableEvent)
	void BI_OnGoalReached();

#pragma endregion

#pragma region Debug

protected:
	/** Displays a path in world space using colored boxes, increasing in size down the path */
	void DisplayDebugPath(TArray<FIntVector2D> ValidPath) const;

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

#pragma endregion

#pragma region Goal Variables

protected:
	/** Whether the mouse has reached the other side of the grid */
	UPROPERTY(BlueprintReadOnly)
	bool bGoalReached = false;

#pragma endregion

};
