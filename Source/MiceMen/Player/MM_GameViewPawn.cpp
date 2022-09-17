// Copyright Alex Coultas, Mice Men Example Project


#include "MM_GameViewPawn.h"

#include "CineCameraComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "Player/MM_PlayerController.h"
#include "Gameplay/MM_ColumnControl.h"
#include "Base/MM_GameMode.h"
#include "Grid/MM_GridManager.h"
#include "MiceMen.h"

// Sets default values
AMM_GameViewPawn::AMM_GameViewPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	SetRootComponent(SceneRoot);

	GameCamera = CreateDefaultSubobject<UCineCameraComponent>(TEXT("Game Camera"));
	GameCamera->SetupAttachment(RootComponent);
}

// Called to bind functionality to input
void AMM_GameViewPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(FName("Grab"), IE_Pressed, this, &AMM_GameViewPawn::BeginGrab);
	PlayerInputComponent->BindAction(FName("Grab"), IE_Released, this, &AMM_GameViewPawn::EndGrab);

}


// Called when the game starts or when spawned
void AMM_GameViewPawn::BeginPlay()
{
	Super::BeginPlay();
	
	// Check gamemode is set
	GetGamemode();
}

void AMM_GameViewPawn::PossessedBy(AController* _NewController)
{
	Super::PossessedBy(_NewController);

	MMPlayerController = Cast<AMM_PlayerController>(_NewController);
}

void AMM_GameViewPawn::BeginTurn()
{
	bTurnActive = true;

	// Store available columns
	if (GetGridManager() && MMPlayerController)
	{
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GameViewPawn::BeginTurn | Beginning turn for %i as %s"), MMPlayerController->GetCurrentTeam(), *MMPlayerController->GetName());

		// Get available column indexes for this player
		TArray<int> AvailableColumns = GridManager->GetTeamColumns(MMPlayerController->GetCurrentTeam());
		int FallbackColumn = -1;

		// For each available column
		for (int Column : AvailableColumns)
		{
			FallbackColumn = Column;

			// Check if this column was already moved a specific amount of times in a row
			if (Column == LastMovedColumn && SameMovedColumnCount >= SameColumnMax)
			{
				// Don't add column to movable
				continue;
			}

			AddColumnAsGrabbable(Column);
			UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GameViewPawn::BeginTurn | Available column %i"), Column);
		}

		// If no available columns, use fall back,
		// for situations such as when all mice are on the same column but it was moved more than the max
		if (CurrentColumnControls.Num() <= 0 && FallbackColumn >= 0)
		{
			AddColumnAsGrabbable(FallbackColumn);
			UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GameViewPawn::BeginTurn | Using fallback column %i"), FallbackColumn);
		}

		// If player is AI, auto take the turn
		if (MMPlayerController->IsAI())
			TakeRandomTurn();
	}

}

void AMM_GameViewPawn::TakeRandomTurn()
{
	int RandomIndex = FMath::RandRange(0, CurrentColumnControls.Num() - 1);
	CurrentColumn = CurrentColumnControls[RandomIndex];
	int RandomDirection = FMath::RandBool() ? 1 : -1;
	if (CurrentColumn)
	{
		FVector NewLocation = CurrentColumn->GetActorLocation();
		NewLocation.Z += RandomDirection * GridManager->GridElementHeight;

		// Don't use pawn grab since its not based on mouse/input
		CurrentColumn->BeginGrab();
		CurrentColumn->UpdatePreviewLocation(NewLocation);
		// If testing mode, instantly move column
		if (GetGamemode() && MMGameMode->GetCurrentGameType() == EGameType::E_TEST)
		{
			CurrentColumn->SetActorLocation(NewLocation);
		}
		EndGrab();
	}
}

void AMM_GameViewPawn::AddColumnAsGrabbable(int Column)
{
	// Get all columns
	TMap<int, AMM_ColumnControl*> AllColumnControls = GridManager->GetColumnControls();

	// Check column controls has the index and that it is a valid pointer
	if (AllColumnControls.Contains(Column) && AllColumnControls[Column])
	{
		// Display column as grabbable and store
		AllColumnControls[Column]->DisplayGrabbable(true, MMPlayerController->GetCurrentTeam());
		CurrentColumnControls.Add(AllColumnControls[Column]);
	}
}

// Called every frame
void AMM_GameViewPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bTurnActive)
		HandleGrab();
}

void AMM_GameViewPawn::BeginGrab()
{
	if (!MMPlayerController || !bTurnActive)
	{
		UE_LOG(MiceMenEventLog, Warning, TEXT("AMM_GameViewPawn::BeginGrab | Failed as turn is not active"));
		return;
	}

	// Deproject mouse cursor to world
	FVector WorldLocation, WorldDirection;
	if (!MMPlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		// Failed to deproject
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GameViewPawn::BeginGrab | Failed to deproject mouse"));
		return;
	}

	// Get hit at mouse position in world on interactable channel
	FHitResult InteractHit;
	if (!GetWorld()->LineTraceSingleByChannel(InteractHit, WorldLocation, WorldLocation + WorldDirection * InteractTraceDistance, ECC_GameTraceChannel1))
	{
		// Failed hit
		UE_LOG(MiceMenEventLog, Warning, TEXT("AMM_GameViewPawn::BeginGrab | Did not hit a column"));
		return;
	}

	AMM_ColumnControl* NewColumn = Cast<AMM_ColumnControl>(InteractHit.GetActor());
	if (NewColumn)
	{
		// Check valid column
		if (CurrentColumnControls.Contains(NewColumn))// GetGridManager() && GridManager->IsTeamInColumn(CurrentColumn->GetControllingColumn(), MMPlayerController->GetCurrentTeam()))
		{
			CurrentColumn = NewColumn;
			UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GameViewPawn::BeginGrab | Begin grabbing column %i"), CurrentColumn->GetColumnIndex());
			CurrentColumn->BeginGrab();
			HitColumnOffset = CurrentColumn->GetActorLocation() - InteractHit.Location;
		}
		else
			UE_LOG(MiceMenEventLog, Warning, TEXT("AMM_GameViewPawn::BeginGrab | Actor hit was not a team column"));
	}
	else
	{
		UE_LOG(MiceMenEventLog, Warning, TEXT("AMM_GameViewPawn::BeginGrab | Actor hit was not a column"));
	}
}

void AMM_GameViewPawn::EndGrab()
{
	if (CurrentColumn)
	{
		// Stop allowing movement until column event complete
		bTurnActive = false;

		// Rebind turn ended event
		// TODO: Improve binding
		if (CurrentColumnDelegateHandle.IsValid())
			CurrentColumnDelegateHandle.Reset();
		// Bind new delegate for column movement
		CurrentColumnDelegateHandle = CurrentColumn->AdjustCompleteDelegate.AddUObject(this, &AMM_GameViewPawn::ColumnAdjusted);
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GameViewPawn::EndGrab | Bind event to column on complete for %i as %s"), MMPlayerController->GetCurrentTeam(), *MMPlayerController->GetName());

		CurrentColumn->EndGrab();

		// The column is being moved to a new slot
		if (CurrentColumn->GetCurrentColumnDirection() != 0)
		{
			int CurrentColumnIndex = CurrentColumn->GetColumnIndex();

			// If the column moved was not the same as the last
			if (CurrentColumnIndex != LastMovedColumn)
			{
				// Store column moved and reset counter (has moved once counting this turn)
				LastMovedColumn = CurrentColumnIndex;
				SameMovedColumnCount = 1;
			}
			// Same column moved, increment counter
			else
			{
				SameMovedColumnCount++;
				UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GameViewPawn::EndGrab | Moved same column at %i time(s)"), SameMovedColumnCount);
			}
		}
	}
	CurrentColumn = nullptr;
}

void AMM_GameViewPawn::HandleGrab()
{
	if (!CurrentColumn || !MMPlayerController)
		return;

	// Deproject mouse cursor to world
	FVector WorldLocation, WorldDirection;
	if (!MMPlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
		// Failed to deproject
		return;

	float T;
	FVector IntersectionLocation;
	if (!UKismetMathLibrary::LinePlaneIntersection_OriginNormal(WorldLocation, WorldLocation + WorldDirection * InteractTraceDistance, CurrentColumn->GetActorLocation(), CurrentColumn->GetActorForwardVector() * -1, T, IntersectionLocation))
		return;

	FVector NewLocation = CurrentColumn->GetActorLocation();
	NewLocation.Z = IntersectionLocation.Z + HitColumnOffset.Z;
	CurrentColumn->UpdatePreviewLocation(NewLocation);
}

void AMM_GameViewPawn::ColumnAdjusted(bool _TurnComplete)
{
	if (_TurnComplete)
	{
		TurnEnded();
	}
	else
	{
		bTurnActive = true;
	}
}

void AMM_GameViewPawn::TurnEnded()
{
	bTurnActive = false;

	for (AMM_ColumnControl* Column : CurrentColumnControls)
	{
		if (Column)
		{
			Column->DisplayGrabbable(false);
			Column->AdjustCompleteDelegate.RemoveAll(this);
		}
	}

	CurrentColumnControls.Empty();


	/*if (MMPlayerController)
	{
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GameViewPawn::TurnEnded | Turn ended for %i as %s"), MMPlayerController->GetCurrentTeam(), *MMPlayerController->GetName());

		MMPlayerController->EndTurn();
	}*/
}

AMM_GridManager* AMM_GameViewPawn::GetGridManager()
{
	if (IsValid(GridManager))
		return GridManager;

	if (!GetGamemode())
		return nullptr;

	GridManager = MMGameMode->GetGridManager();


	return GridManager;
}

AMM_GameMode* AMM_GameViewPawn::GetGamemode()
{
	if (MMGameMode)
		return MMGameMode;

	MMGameMode = GetWorld()->GetAuthGameMode<AMM_GameMode>();

	return MMGameMode;
}