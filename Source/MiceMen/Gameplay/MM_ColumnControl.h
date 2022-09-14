// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MM_ColumnControl.generated.h"

/**
 * Event for when a move has been made on the column,
 * bool bTurnComplete determines whether a turn has completed, or was canceled.
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FColumnAdjustCompleteDelegate, bool);

UCLASS()
class MICEMEN_API AMM_ColumnControl : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMM_ColumnControl();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UBoxComponent* GrabbableBox;

	void SetupColumn(int _ColumnID, class AMM_GridManager* _GridManager);

	void BeginGrab();
	void UpdatePreviewLocation(FVector _NewLocation);
	void EndGrab();

	void UpdateCollumn();

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float LerpSpeed = 10.0f;

	FColumnAdjustCompleteDelegate AdjustCompleteDelegate;


protected:
	UPROPERTY(BlueprintReadOnly)
	class AMM_GridManager* GridManager;

	UPROPERTY(BlueprintReadOnly)
		float GridElementHeight = 100.0f;
	UPROPERTY(BlueprintReadOnly)
		float SnapSize = 40.0f;

	int ControllingColumn = -1;

	int CurrentDirectionChange = 0;

	bool bGrabbed = false;
	bool bLerp = false;
	FVector PreviewLocation;
	FVector OriginalColumnLocation;
	float ColumnHeight;
};
