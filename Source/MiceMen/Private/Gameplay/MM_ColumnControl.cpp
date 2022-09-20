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

void AMM_ColumnControl::SetupColumn(int _ColumnID, AMM_GridManager* _GridManager)
{
	// Store variables
	ControllingColumn = _ColumnID;
	GridManager = _GridManager;
	GridElementHeight = GridManager->GridElementHeight;

	// Calculate sizes
	ColumnHeight = GridElementHeight * _GridManager->GetGridSize().Y;
	const FVector BoxSize = FVector(50, _GridManager->GridElementWidth / 2.0f, ColumnHeight / 2.0);
	
	// Apply sizes
	GrabbableBox->SetBoxExtent(BoxSize);
	GrabbableBox->SetRelativeLocation(FVector(0, 0, ColumnHeight / 2));
}

void AMM_ColumnControl::BeginPlay()
{
	Super::BeginPlay();	
}

void AMM_ColumnControl::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Grabbed or lerping
	if (bGrabbed || bLerp)
	{
		// Lerp position towards preview location
		const float LerpAlpha = FMath::Clamp(LerpSpeed * DeltaTime, 0.0f, 1.0f);
		const FVector NewLocation = FMath::Lerp(GetActorLocation(), PreviewLocation, LerpAlpha);
		SetActorLocation(NewLocation);
	}
	
	// If column has been released but is still lerping, check if close enough to snap
	if (!bGrabbed && bLerp && abs(GetActorLocation().Z - PreviewLocation.Z) < 1.0f)
	{
		LockInColumn();
	}
}

bool AMM_ColumnControl::BeginGrab()
{
	// Cannot grab while in motion
	if (bGrabbed || bLerp)
	{
		return false;
	}

	// Initial grab variables
	bGrabbed = true;
	bLerp = true;
	OriginalColumnLocation = GetActorLocation();
	PreviewLocation = OriginalColumnLocation;

	BI_BeginGrab();

	return true;
}

void AMM_ColumnControl::UpdatePreviewLocation(FVector _NewLocation)
{
	PreviewLocation = _NewLocation;

	EDirection NewDirectionChange = EDirection::E_NONE;

	const float MaxVerticalLocation = OriginalColumnLocation.Z + GridElementHeight;
	const float MinVerticalLocation = OriginalColumnLocation.Z - GridElementHeight;

	// Limit movement to only one grid slot above or below original location
	PreviewLocation.Z = FMath::Clamp(PreviewLocation.Z, MinVerticalLocation, MaxVerticalLocation);

	// Snap when close to grid slot
	if (abs(_NewLocation.Z - OriginalColumnLocation.Z) < SnapSize)
	{
		PreviewLocation.Z = OriginalColumnLocation.Z;
		NewDirectionChange = EDirection::E_NONE;
	}
	// Snap upwards
	else if (_NewLocation.Z > MaxVerticalLocation - SnapSize)
	{
		PreviewLocation.Z = MaxVerticalLocation;
		NewDirectionChange = EDirection::E_UP;
	}
	// Snap downwards
	// TODO Check it should be plus SnapSize and not minus
	else if (_NewLocation.Z < MinVerticalLocation + SnapSize)
	{
		PreviewLocation.Z = MinVerticalLocation;
		NewDirectionChange = EDirection::E_DOWN;
	}

	// Check position is new slot and update
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
	if (CurrentDirectionChange != EDirection::E_NONE)
	{
		const int DirectionMultiplier = CurrentDirectionChange == EDirection::E_UP ? 1 : -1;
		PreviewLocation.Z = OriginalColumnLocation.Z + GridElementHeight * DirectionMultiplier;
	}
	else
	{
		PreviewLocation.Z = OriginalColumnLocation.Z;
	}

	BI_EndGrab();
}

void AMM_ColumnControl::LockInColumn()
{
	// Lock location and stop lerping movement
	SetActorLocation(PreviewLocation);
	bLerp = false;

	// Column movement complete, turn has ended if the direction was changed ie not none
	AdjustCompleteDelegate.Broadcast(CurrentDirectionChange != EDirection::E_NONE);

	// New direction chosen, update grid manager
	if (CurrentDirectionChange != EDirection::E_NONE)
	{
		// Move base column location back to original, keeping the attached children in the same position
		ResetToDefaultPosition();
		if (GridManager)
		{
			GridManager->AdjustColumn(ControllingColumn, CurrentDirectionChange);
		}
		else
		{
			UE_LOG(MiceMenEventLog, Error, TEXT("AMM_ColumnControl::UpdateCollumn | Grid Manager null for %i named %s"), ControllingColumn, *GetName());
		}
	}
}

void AMM_ColumnControl::ResetToDefaultPosition()
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
	SetActorLocation(OriginalColumnLocation);

	// Reattach Children
	for (AActor* AttachedActor : AttachedActors)
	{
		AttachedActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}
}

void AMM_ColumnControl::DisplayAsGrabbable(bool _bGrabbable, ETeam _Team /* = ETeam::E_NONE*/)
{
	BI_OnDisplayAsGrabbable(_bGrabbable, _Team);
}

void AMM_ColumnControl::BN_DirectionChanged_Implementation(EDirection _NewDirection)
{

}

