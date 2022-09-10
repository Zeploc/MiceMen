// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MM_ColumnControl.generated.h"

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

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FVector GetOriginalColumnLocation() {
		return OriginalColumnLocation;			
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveColumnBackToOriginalPosition();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float LerpSpeed = 10.0f;


protected:
	UPROPERTY(BlueprintReadOnly)
	class AMM_GridManager* GridManager;

	UPROPERTY(BlueprintReadOnly)
		float MaxPullAmount = 100.0f;
	UPROPERTY(BlueprintReadOnly)
		float SnapSize = 40.0f;

	int ControllingColumn = -1;

	bool bGrabbed = false;
	bool bLerp = false;
	FVector PreviewLocation;
	FVector OriginalColumnLocation;
	float ColumnHeight;
};
