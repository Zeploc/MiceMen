// Copyright Alex Coultas, Mice Men Example Project


#include "Gameplay/MM_ColumnControl.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Components/BoxComponent.h"

#include "Grid/MM_GridManager.h"
#include "MiceMen.h"

// Sets default values
AMM_ColumnControl::AMM_ColumnControl()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	SetRootComponent(SceneRoot);

	GrabbableBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Grabbable Box"));
	GrabbableBox->SetupAttachment(RootComponent);
	GrabbableBox->SetCollisionProfileName(FName("GridColumn"));
}

// Called when the game starts or when spawned
void AMM_ColumnControl::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMM_ColumnControl::MoveColumnBackToOriginalPosition()
{
	// Get attached actors
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);

	// Detach Children
	for (AActor* AttachedActor : AttachedActors)
	{
		AttachedActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
	// Reset position
	//TODO: NOT WORKING???
	SetActorLocation(OriginalColumnLocation);
	// Reattach
	for (AActor* AttachedActor : AttachedActors)
	{
		AttachedActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}
}

void AMM_ColumnControl::BN_DirectionChanged_Implementation(int _NewDirection)
{

}

void AMM_ColumnControl::SetupColumn(int _ColumnID, AMM_GridManager* _GridManager)
{
	ControllingColumn = _ColumnID;
	GridManager = _GridManager;

	ColumnHeight = _GridManager->GridElementHeight * _GridManager->GetGridSize().Y;
	FVector BoxSize = FVector(50, _GridManager->GridElementWidth / 2.0f, ColumnHeight / 2.0);
	
	GrabbableBox->SetBoxExtent(BoxSize);
	GrabbableBox->SetRelativeLocation(FVector(0, 0, ColumnHeight / 2));

	GridElementHeight = GridManager->GridElementHeight;

}

void AMM_ColumnControl::BeginGrab()
{
	// Cannnot grab while in movement
	if (bGrabbed || bLerp)
		return;

	bGrabbed = true;
	bLerp = true;
	OriginalColumnLocation = GetActorLocation();
	PreviewLocation = OriginalColumnLocation;

	BI_BeginGrab();
}

void AMM_ColumnControl::UpdatePreviewLocation(FVector _NewLocation)
{
	PreviewLocation = _NewLocation;

	int NewDirectionChange = 0;

	// Limit to only one grid slot movement
	PreviewLocation.Z = FMath::Clamp(PreviewLocation.Z, OriginalColumnLocation.Z - GridElementHeight, OriginalColumnLocation.Z + GridElementHeight);
	// Snap when close to grid slot
	if (abs(_NewLocation.Z - OriginalColumnLocation.Z) < SnapSize)
	{
		PreviewLocation.Z = OriginalColumnLocation.Z;
		NewDirectionChange = 0;
	}
	else if (_NewLocation.Z > OriginalColumnLocation.Z + (GridElementHeight - SnapSize))
	{
		PreviewLocation.Z = OriginalColumnLocation.Z + GridElementHeight;
		NewDirectionChange = 1;
	}
	else if (_NewLocation.Z < OriginalColumnLocation.Z - (GridElementHeight - SnapSize))
	{
		PreviewLocation.Z = OriginalColumnLocation.Z - GridElementHeight;
		NewDirectionChange = -1;
	}

	if (CurrentDirectionChange != NewDirectionChange)
	{
		CurrentDirectionChange = NewDirectionChange;
		BN_DirectionChanged(CurrentDirectionChange);
	}
}

void AMM_ColumnControl::EndGrab()
{
	bGrabbed = false;

	// Set snap to point based on current direction
	if (CurrentDirectionChange != 0)
	{
		PreviewLocation.Z = OriginalColumnLocation.Z + GridElementHeight * CurrentDirectionChange;
	}
	else
	{
		PreviewLocation.Z = OriginalColumnLocation.Z;
	}

	BI_EndGrab();
}

void AMM_ColumnControl::UpdateCollumn()
{

	// Column move complete, turn has ended if the direction was changed ie not 0
	AdjustCompleteDelegate.Broadcast(CurrentDirectionChange != 0);

	// New direction chosen, update grid manager
	if (CurrentDirectionChange != 0)
	{
		MoveColumnBackToOriginalPosition();
		if (GridManager)
			GridManager->AdjustColumn(ControllingColumn, CurrentDirectionChange);
		else
		{
			UE_LOG(MiceMenEventLog, Error, TEXT("AMM_ColumnControl::UpdateCollumn | Grid Manager null for %i as %s"), ControllingColumn, *GetName());
		}

	}

}

void AMM_ColumnControl::DisplayGrabbable(bool _bGrabbable, int _Team /* = -1*/)
{
	BI_DisplayGrabbable(_bGrabbable, _Team);
}

// Called every frame
void AMM_ColumnControl::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If column has been released but is still lerping, check if close enough to snap
	if (!bGrabbed && bLerp && abs(GetActorLocation().Z - PreviewLocation.Z) < 1.0f)
	{
		SetActorLocation(PreviewLocation);
		bLerp = false;
		UpdateCollumn();
	}

	// Grabbed or lerp on
	if (bGrabbed || bLerp)
	{
		// Lerp position
		FVector NewLocation = FMath::Lerp(GetActorLocation(), PreviewLocation, LerpSpeed * DeltaTime);
		SetActorLocation(NewLocation);
	}

	
}

