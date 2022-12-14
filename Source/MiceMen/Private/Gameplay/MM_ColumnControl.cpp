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

void AMM_ColumnControl::SetupColumn(int InColumnID, AMM_GridManager* InGridManager)
{
	// Store variables
	ControllingIndex = InColumnID;
	GridManager = InGridManager;
	GridElementHeight = GridManager->GridElementHeight;

	// Calculate sizes
	ColumnHeight = GridElementHeight * InGridManager->GetGridSize().Y;
	const FVector BoxSize = FVector(50, InGridManager->GridElementWidth / 2.0f, ColumnHeight / 2.0);

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
	OriginalLocation = GetActorLocation();
	PreviewLocation = OriginalLocation;

	BI_BeginGrab();

	return true;
}

void AMM_ColumnControl::UpdatePreviewLocation(const FVector& NewLocation)
{
	PreviewLocation = NewLocation;

	EDirection NewDirectionChange = EDirection::E_NONE;

	const float MaxVerticalLocation = OriginalLocation.Z + GridElementHeight;
	const float MinVerticalLocation = OriginalLocation.Z - GridElementHeight;

	// Limit movement to only one grid slot above or below original location
	PreviewLocation.Z = FMath::Clamp(PreviewLocation.Z, MinVerticalLocation, MaxVerticalLocation);

	// Snap when close to grid slot
	if (abs(NewLocation.Z - OriginalLocation.Z) < SnapSize)
	{
		PreviewLocation.Z = OriginalLocation.Z;
		NewDirectionChange = EDirection::E_NONE;
	}
	// Snap upwards
	else if (NewLocation.Z > MaxVerticalLocation - SnapSize)
	{
		PreviewLocation.Z = MaxVerticalLocation;
		NewDirectionChange = EDirection::E_UP;
	}
	// Snap downwards
	else if (NewLocation.Z < MinVerticalLocation + SnapSize)
	{
		PreviewLocation.Z = MinVerticalLocation;
		NewDirectionChange = EDirection::E_DOWN;
	}

	// Check position is new slot and update
	if (CurrentDirectionChange != NewDirectionChange)
	{
		UE_LOG(MiceMenEventLog, Log, TEXT("AMM_ColumnControl::UpdatePreviewLocation | Updating Direction to %s"),
			NewDirectionChange == EDirection::E_NONE ? TEXT("None") : NewDirectionChange == EDirection::E_DOWN ?
			TEXT("Down"):
			TEXT("Up"));
		CurrentDirectionChange = NewDirectionChange;
		BN_DirectionChanged(CurrentDirectionChange);
	}
}

void AMM_ColumnControl::EndGrab()
{
	bGrabbed = false;
	bLerp = true;

	// Set snap to point based on current direction
	if (CurrentDirectionChange != EDirection::E_NONE)
	{
		UE_LOG(MiceMenEventLog, Log, TEXT("AMM_ColumnControl::EndGrab | Direction change for column %i with name %s and lerp is %s"), ControllingIndex, *GetName(), bLerp ? TEXT("true") : TEXT("false"));
		const int DirectionMultiplier = CurrentDirectionChange == EDirection::E_UP ? 1 : -1;
		PreviewLocation.Z = OriginalLocation.Z + GridElementHeight * DirectionMultiplier;
	}
	else
	{
		UE_LOG(MiceMenEventLog, Log, TEXT("AMM_ColumnControl::EndGrab | No direction change for column %i"), ControllingIndex);
		PreviewLocation.Z = OriginalLocation.Z;
	}

	BI_EndGrab();
}

void AMM_ColumnControl::LockInColumn()
{
	UE_LOG(MiceMenEventLog, Log, TEXT("AMM_ColumnControl::LockInColumn | Locking in column %i"), ControllingIndex);

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
			GridManager->AdjustColumn(ControllingIndex, CurrentDirectionChange);
			// Start processing mice based on column change
			GridManager->BeginProcessMice();
		}
		else
		{
			UE_LOG(MiceMenEventLog, Error, TEXT("AMM_ColumnControl::UpdateCollumn | Grid Manager null for %i named %s"), ControllingIndex, *GetName());
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
	SetActorLocation(OriginalLocation);

	// Reattach Children
	for (AActor* AttachedActor : AttachedActors)
	{
		AttachedActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}
}

void AMM_ColumnControl::DisplayAsGrabbable(bool bGrabbable, ETeam Team /* = ETeam::E_NONE*/)
{
	BI_OnDisplayAsGrabbable(bGrabbable, Team);
}

void AMM_ColumnControl::BN_DirectionChanged_Implementation(EDirection NewDirection)
{
}
