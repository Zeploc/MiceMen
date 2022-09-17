// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

	/** Sets initial values and sizes for the column */
	void SetupColumn(int _ColumnID, AMM_GridManager* _GridManager);

	/** Called when the player interacts with the column, sets initial values, returns true if successful */
	bool BeginGrab();
	/**
	* Called when the grabbed location has been moved by the player,
	* used to visually update the position of the column. 
	* @param _NewLocation the goal location of the column
	*/
	void UpdatePreviewLocation(FVector _NewLocation);

	/** Called when the player released the column, will update the preview position */
	void EndGrab();

	/** Locks the column into the slot, calling events to update the grid elements */
	void LockInCollumn();

	/** Toggles whether the column should display as interactable, and for which team */
	void DisplayGrabbable(bool _bGrabbable, int _Team = -1);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FVector GetOriginalColumnLocation() {
		return OriginalColumnLocation;			
	}
	int GetColumnIndex() {
		return ControllingColumn;
	}
	int GetCurrentColumnDirection() {
		return CurrentDirectionChange;
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/**
	* Moves column base position to original position relative to the grid. 
	* Will reattach the grid elements to maintain their transforms
	*/
	void MoveColumnBackToOriginalPosition();

	UFUNCTION(BlueprintImplementableEvent)
		void BI_DisplayGrabbable(bool _bGrabbable, int _Team);
	UFUNCTION(BlueprintImplementableEvent)
		void BI_BeginGrab();
	UFUNCTION(BlueprintImplementableEvent)
		void BI_EndGrab();

	UFUNCTION(BlueprintNativeEvent)
		void BN_DirectionChanged(int _NewDirection);

public:
	/** The speed the column lerps to the preview location (dragged or released) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float LerpSpeed = 10.0f;

	/** Called when the column slots into place */
	FColumnAdjustCompleteDelegate AdjustCompleteDelegate;


protected:
	UPROPERTY(BlueprintReadOnly)
	AMM_GridManager* GridManager;

	/** The stored height for a grid element, retrieved from the grid manager */
	UPROPERTY(BlueprintReadOnly)
		float GridElementHeight = 100.0f;
	/** The distance away from a slot which the column will snap to when held */
	UPROPERTY(BlueprintReadOnly)
		float SnapSize = 40.0f;

	/** The column index linked to the x axis on the grid */
	int ControllingColumn = -1;

	/**
	 * The active grid slot change, 1 for up, -1 for down, 0 for no change.
	 * Updated while the player interacts with the column
	 */
	int CurrentDirectionChange = 0;

	/** True when the column is held by the player */
	bool bGrabbed = false;
	/** The column is actively lerping to the preview position */
	bool bLerp = false;
	/** The location the column will lerp towards */
	FVector PreviewLocation;
	/** The initial column position before it was moved, used to return back to */
	FVector OriginalColumnLocation;
	/** The full column height in world space */
	float ColumnHeight;
};
