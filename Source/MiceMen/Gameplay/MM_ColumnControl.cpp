// Copyright Alex Coultas, Mice Men Example Project


#include "Gameplay/MM_ColumnControl.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Components/BoxComponent.h"

#include "Grid/MM_GridManager.h"

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

void AMM_ColumnControl::SetupColumn(int _ColumnID, AMM_GridManager* _GridManager)
{
	ControllingColumn = _ColumnID;
	GridManager = _GridManager;

	ColumnHeight = _GridManager->GridElementHeight * _GridManager->GetGridSize().Y;
	FVector BoxSize = FVector(50, _GridManager->GridElementWidth / 2.0f, ColumnHeight / 2.0);
	
	GrabbableBox->SetBoxExtent(BoxSize);
	GrabbableBox->SetRelativeLocation(FVector(0, 0, ColumnHeight / 2));

	MaxPullAmount = GridManager->GridElementHeight;

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

	// Limit to only one grid slot movement
	PreviewLocation.Z = FMath::Clamp(PreviewLocation.Z, OriginalColumnLocation.Z - MaxPullAmount, OriginalColumnLocation.Z + MaxPullAmount);
	// Snap when close to grid slot
	if (abs(_NewLocation.Z - OriginalColumnLocation.Z) < SnapSize)
		PreviewLocation.Z = OriginalColumnLocation.Z;
	else if (abs(_NewLocation.Z - OriginalColumnLocation.Z - MaxPullAmount) < SnapSize)
		PreviewLocation.Z = OriginalColumnLocation.Z + MaxPullAmount;
	else if (abs(_NewLocation.Z - OriginalColumnLocation.Z + MaxPullAmount) < SnapSize)
		PreviewLocation.Z = OriginalColumnLocation.Z - MaxPullAmount;
}

void AMM_ColumnControl::EndGrab()
{
	bGrabbed = false;

	// Find out which snapping point is closest to to lerp to
	if (abs(PreviewLocation.Z - OriginalColumnLocation.Z - MaxPullAmount) < SnapSize)
	{
		PreviewLocation.Z = OriginalColumnLocation.Z + MaxPullAmount;
	}
	else if (abs(PreviewLocation.Z - OriginalColumnLocation.Z + MaxPullAmount) < SnapSize)
	{
		PreviewLocation.Z = OriginalColumnLocation.Z - MaxPullAmount;
	}
	else
	{
		PreviewLocation.Z = OriginalColumnLocation.Z;
	}

	BI_EndGrab();
}

void AMM_ColumnControl::UpdateCollumn()
{
	int DirectionChange = 0;

	// Find out which snapping point is closest to for adjusting column
	if (abs(PreviewLocation.Z - OriginalColumnLocation.Z - MaxPullAmount) < SnapSize)
	{
		DirectionChange = 1;
	}
	else if (abs(PreviewLocation.Z - OriginalColumnLocation.Z + MaxPullAmount) < SnapSize)
	{
		DirectionChange = -1;
	}

	// New direction chosen, update grid manager
	if (DirectionChange != 0)
	{
		MoveColumnBackToOriginalPosition();
		GridManager->AdjustColumn(ControllingColumn, DirectionChange);

		AdjustCompleteDelegate.Broadcast();
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

