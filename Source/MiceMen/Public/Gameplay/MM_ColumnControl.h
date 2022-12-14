// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Base/MM_GameEnums.h"
#include "Base/MM_GridEnums.h"
#include "MM_ColumnControl.generated.h"

/**
 * Event for when a move has been made on the column,
 * @bool bTurnComplete determines whether a turn has completed, or was canceled.
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FColumnAdjustCompleteDelegate, bool);

class USceneComponent;
class UBoxComponent;
class AMM_GridManager;

/**
 * The main control for a column, which the player interacts with
 * Grid elements attach to this for moving with the column
 */
UCLASS()
class MICEMEN_API AMM_ColumnControl : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMM_ColumnControl();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* GrabbableBox;

#pragma region Core

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#pragma endregion

#pragma region Management

public:
	/** Sets initial values and sizes for the column */
	virtual void SetupColumn(int InColumnID, AMM_GridManager* InGridManager);

	UFUNCTION(BlueprintPure)
	int GetColumnIndex() const { return ControllingIndex; }

protected:
	/** Locks the column into the slot, calling events to update the grid elements */
	void LockInColumn();

#pragma endregion

#pragma region Location

public:
	/**
	* Called when the grabbed location has been moved by the player,
	* used to visually update the position of the column. 
	* @param NewLocation the goal location of the column
	*/
	virtual void UpdatePreviewLocation(const FVector& NewLocation);

	UFUNCTION(BlueprintPure)
	FVector GetOriginalLocation() const { return OriginalLocation; }

	UFUNCTION(BlueprintPure)
	EDirection GetCurrentDirection() const { return CurrentDirectionChange; }

protected:
	UFUNCTION(BlueprintNativeEvent)
	void BN_DirectionChanged(EDirection NewDirection);

	/**
	* Moves column position to the original position relative to the grid.
	* Will reattach the grid elements to maintain their transforms
	*/
	virtual void ResetToDefaultPosition();

#pragma endregion

#pragma region Interaction

public:
	/** Called when the player interacts with the column, sets initial values, returns true if successful */
	virtual bool BeginGrab();

	/** Called when the player released the column, will update the preview position */
	virtual void EndGrab();

	/** Toggles whether the column should display as interactable, and for which team */
	void DisplayAsGrabbable(bool bGrabbable, ETeam Team = ETeam::E_NONE);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void BI_OnDisplayAsGrabbable(bool bGrabbable, ETeam Team);

	UFUNCTION(BlueprintImplementableEvent)
	void BI_BeginGrab();

	UFUNCTION(BlueprintImplementableEvent)
	void BI_EndGrab();

#pragma endregion

//-------------------------------------------------------

#pragma region Location Variables

public:
	/** The speed the column lerps to the preview location (dragged or released) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LerpSpeed = 10.0f;

	/** The distance away from a slot which the column will snap to when held */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SnapSize = 40.0f;

protected:
	/**
	 * The active grid slot change, 1 for up, -1 for down, 0 for no change.
	 * Updated while the player interacts with the column
	 */
	EDirection CurrentDirectionChange = EDirection::E_NONE;

	/** The column is actively lerping to the preview position */
	UPROPERTY(BlueprintReadOnly)
	bool bLerp = false;

	/** The location the column will lerp towards */
	UPROPERTY(BlueprintReadOnly)
	FVector PreviewLocation;

	/** The initial column position before it was moved, used to return back to */
	UPROPERTY(BlueprintReadOnly)
	FVector OriginalLocation;

#pragma endregion

#pragma region Interaction Variables

protected:
	/** True when the column is held by the player */
	UPROPERTY(BlueprintReadOnly)
	bool bGrabbed = false;

#pragma endregion

#pragma region Management Variables

public:
	/** Called when the column slots into place */
	FColumnAdjustCompleteDelegate AdjustCompleteDelegate;

protected:
	/** The column index linked to the x axis on the grid */
	int ControllingIndex = -1;

	/** The full column height in world space */
	UPROPERTY(BlueprintReadOnly)
	float ColumnHeight;

#pragma endregion

#pragma region Grid Variables

protected:
	UPROPERTY(BlueprintReadOnly)
	AMM_GridManager* GridManager;

	/** The stored height for a grid element, retrieved from the grid manager */
	UPROPERTY(BlueprintReadOnly)
	float GridElementHeight = 100.0f;

#pragma endregion
};
