// Copyright Alex Coultas, Mice Men Example Project


#include "Grid/MM_GridManager.h"

#include "Kismet/GameplayStatics.h"

#include "MM_GridElement.h"
#include "MM_GridBlock.h"
#include "Gameplay/MM_Mouse.h"
#include "Gameplay/MM_ColumnControl.h"
#include "MM_GridObject.h"
#include "MiceMen.h"
#include "MM_GameMode.h"
#include "MM_PlayerController.h"

// Sets default values
AMM_GridManager::AMM_GridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GridSize = FIntVector2D(19, 13);
	GridBlockClass = AMM_GridBlock::StaticClass();
	MouseClass = AMM_Mouse::StaticClass();
	ColumnControlClass = AMM_ColumnControl::StaticClass();
}

// Called when the game starts or when spawned
void AMM_GridManager::BeginPlay()
{
	Super::BeginPlay();

}
// Called every frame
void AMM_GridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bDebugGridEnabled)
		DebugVisualiseGrid();
}


void AMM_GridManager::SetupGrid(FIntVector2D _GridSize, AMM_GameMode* _MMGameMode)
{
	GridSize = _GridSize;
	MMGameMode = _MMGameMode;
}

void AMM_GridManager::RebuildGrid(int _InitialMiceCount)
{
	GridCleanUp();

	if (GridSize.X < 2 || GridSize.Y < 2)
	{
		UE_LOG(MiceMenEventLog, Error, TEXT("GRID TOO SMALL, CANNOT SETUP"));
		return;
	}

	GridObject = NewObject<UMM_GridObject>(this, UMM_GridObject::StaticClass());
	GridObject->SetupGrid(GridSize);

	// Remainder of divide by 2, either 0 or 1
	GapSize = GridSize.X % 2;
	// Team size is the amount without the gap, halved
	TeamSize = (GridSize.X - GapSize) / 2;

	PopulateGrid();
	PopulateMice(_InitialMiceCount);
}

void AMM_GridManager::GridCleanUp()
{
	TArray< AMM_ColumnControl*> Columns;
	ColumnControls.GenerateValueArray(Columns);
	for (AMM_ColumnControl* Column : Columns)
	{
		if (Column)
			Column->Destroy();
	}
	ColumnControls.Empty();

	if (GridObject)
		GridObject->CleanUp();
	// Destroy?? GC auto?
	GridObject = nullptr;
}

void AMM_GridManager::PopulateGrid()
{
	// For each column
	for (int x = 0; x < GridSize.X; x++)
	{
		// Check class is valid
		if (!ColumnControlClass)
		{
			ColumnControlClass = AMM_ColumnControl::StaticClass();
		}
		FTransform ColumnTransform = GetWorldTransformFromCoord(FIntVector2D(x, 0));
		AMM_ColumnControl* NewColumnControl = GetWorld()->SpawnActorDeferred<AMM_ColumnControl>(ColumnControlClass, ColumnTransform);
		NewColumnControl->SetupColumn(x, this);
		UGameplayStatics::FinishSpawningActor(NewColumnControl, ColumnTransform);
		ColumnControls.Add(x, NewColumnControl);

		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::PopulateGrid | Adding collumn at %i"), x);

		MouseColumns.Add(x, TArray<AMM_Mouse*>());
		AvailableColumnTeams.Add(x, TArray<int>());

		// For each row, add to column array
		for (int y = 0; y < GridSize.Y; y++)
		{
			// TODO: Rename to "random" place block? better name to define purpose
			PlaceBlock({ x, y }, NewColumnControl);
		}
	}

	// TODO: Remove these once confirmed not necessary
	// Update FreeSlots array
	GridObject->RegenerateFreeSlots();
}

void AMM_GridManager::PlaceBlock(FIntVector2D _NewCoord, AMM_ColumnControl* NewColumnControl)
{
	// Initial variables
	FTransform GridElementTransform = GetWorldTransformFromCoord(_NewCoord);

	// Prepare center pieces
	// TODO IMPROVE LOGIC
	bool IsPresetCenterBlock = _NewCoord.X >= TeamSize - 1 && _NewCoord.X <= TeamSize + GapSize;
	bool isPresetBlockFilled = false;
	if (IsPresetCenterBlock)
	{
		// Center column
		if (_NewCoord.X == TeamSize)
			// If its not every third block
			isPresetBlockFilled = _NewCoord.Y % 3 != 0;
		else if ((_NewCoord.Y + 3) % 3 == 0)
			// If every third block
			isPresetBlockFilled = true;
	}

	// Initial testing random bool for placement
	if ((FMath::RandBool() && !IsPresetCenterBlock) || isPresetBlockFilled)
	{
		// Create new Grid block object 
		AMM_GridBlock* NewGridBlock = GetWorld()->SpawnActor<AMM_GridBlock>(GridBlockClass, GridElementTransform);
		if (!NewGridBlock)
		{
			UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GridManager::PlaceBlock | Failed to create Grid Block!"));
			return;
		}
		NewGridBlock->SetupGridInfo(this, _NewCoord);

		// Add to column array
		if (NewColumnControl)
			NewGridBlock->AttachToActor(NewColumnControl, FAttachmentTransformRules::KeepWorldTransform);
		GridObject->SetGridElement(_NewCoord, NewGridBlock);

		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::PlaceBlock | Adding grid block at %s"), *_NewCoord.ToString());
	}
	else
	{
		// Set element as empty
		GridObject->SetGridElement(_NewCoord, nullptr);
	}
}

void AMM_GridManager::PopulateMice(int _MicePerTeam)
{
	FIntVector2D TeamRanges[] = {
		FIntVector2D(0, TeamSize - 1),
		FIntVector2D(TeamSize + GapSize, GridSize.X - 1)
	};

	for (int iTeam = 0; iTeam < 2; iTeam++)
	{
		// Add initial team array
		TeamMice.Add(iTeam, TArray<AMM_Mouse*>());
		if (MMGameMode)
			MMGameMode->AddTeam(iTeam);
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::PopulateMice | Adding mice for team %i"), iTeam);

		// Add mice for team
		for (int iMouse = 0; iMouse < _MicePerTeam; iMouse++)
		{
			FIntVector2D NewRandomMousePosition = GridObject->GetRandomGridCoordInColumnRange(TeamRanges[iTeam].X, TeamRanges[iTeam].Y);

			// Move to final position
			TArray<FIntVector2D> ValidPath = GridObject->GetValidPath(NewRandomMousePosition, iTeam == 0 ? 1 : -1);
			NewRandomMousePosition = ValidPath.Last();

			// Get coordinates in world
			FTransform GridElementTransform = GetWorldTransformFromCoord(NewRandomMousePosition);

			// Setup new Mouse
			AMM_Mouse* NewMouse = GetWorld()->SpawnActorDeferred<AMM_Mouse>(MouseClass, GridElementTransform);
			NewMouse->SetupGridInfo(this, NewRandomMousePosition);
			NewMouse->SetupMouse(iTeam);
			UGameplayStatics::FinishSpawningActor(NewMouse, GridElementTransform);
			NewMouse->AttachToActor(ColumnControls[NewRandomMousePosition.X], FAttachmentTransformRules::KeepWorldTransform);
			Mice.Add(NewMouse);
			TeamMice[iTeam].Add(NewMouse);
			AddMouseToColumn(NewRandomMousePosition.X, NewMouse);

			GridObject->SetGridElement(NewRandomMousePosition, NewMouse);

			UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::PopulateMice | Adding mice for team %i at %s"), iTeam, *NewRandomMousePosition.ToString());

			// Update FreeSlots array
			GridObject->RegenerateFreeSlots();
		}
	}

}

TArray<FVector> AMM_GridManager::PathFromCoordToWorld(TArray<FIntVector2D> _CoordPath) const
{
	TArray<FVector> NewWorldPath;
	for (FIntVector2D _Coord : _CoordPath)
	{		
		NewWorldPath.Add(GetWorldTransformFromCoord(_Coord).GetLocation());
	}
	return NewWorldPath;
}

FTransform AMM_GridManager::GetWorldTransformFromCoord(FIntVector2D _Coords) const
{
	FTransform GridElementTransform = GetActorTransform();
	FVector NewRelativeLocation = FVector::Zero();

	// X is forward, so horizontal axis is Y, and vertical axis is Z
	NewRelativeLocation.Y -= (GridSize.X / 2) * GridElementWidth;
	NewRelativeLocation.Y += _Coords.X * GridElementWidth;
	NewRelativeLocation.Z += _Coords.Y * GridElementHeight;
	NewRelativeLocation = GetActorTransform().TransformPosition(NewRelativeLocation);

	// Transform location to be relative to grid manager
	GridElementTransform.SetLocation(NewRelativeLocation);

	return GridElementTransform;
}


void AMM_GridManager::BeginProcessMice()
{
	TArray<int> OrderedTeamsToProcess;

	if (MMGameMode && MMGameMode->GetCurrentPlayer())
	{
		// Link to who's turn it is, so their mice move first
		int CurrentPlayerTeam = MMGameMode->GetCurrentPlayer()->GetCurrentTeam();
		OrderedTeamsToProcess.Add(CurrentPlayerTeam);
		// TODO: Improve logic to get other team
		OrderedTeamsToProcess.Add(CurrentPlayerTeam == 0 ? 1 : 0);
	}

	for (int iTeam : OrderedTeamsToProcess)
	{
		// Mice for this team not added
		if (!TeamMice.Contains(iTeam))
			continue;

		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::BeginProcessMice | Processing mice for team %i"), iTeam);

		// TODO: Order need to be more "forward/lower" players, would use more iteration/processing
		// ie FIRST lower players go first, so that higher players fall and can move after
		// THEN forward players go first so they don't block other mice

		TArray<AMM_Mouse*> CurrentTeamMiceToProcess;

		// TODO: Improve Logic
		// NOTE: Storing the Mice and checking/inserting in order, is more efficient than iterating through the whole 
		for (AMM_Mouse* TeamMouse : TeamMice[iTeam])
		{
			// Add first one without iteration
			if (CurrentTeamMiceToProcess.Num() <= 0)
			{
				CurrentTeamMiceToProcess.Add(TeamMouse);
				continue;
			}
			
			FIntVector2D Coordinates = TeamMouse->GetCoordinates();
			for (int i = 0; i < CurrentTeamMiceToProcess.Num(); i++)
			{
				AMM_Mouse* OrderedMouse = CurrentTeamMiceToProcess[i];
				if (Coordinates.Y <= OrderedMouse->GetCoordinates().Y)
				{
					// Check is same row, that the current mouse is more forward
					if (Coordinates.Y == OrderedMouse->GetCoordinates().Y)
					{
						// Check if the mouse is more forward, based on the team
						bool isMoreForward = Coordinates.X > OrderedMouse->GetCoordinates().X;

						// Team is going left
						if (iTeam == 1)
							isMoreForward = Coordinates.X < OrderedMouse->GetCoordinates().X;

						// Mouse is not more forward, go to next
						if (!isMoreForward)
							continue;
					}

					// Insert at current
					CurrentTeamMiceToProcess.Insert(TeamMouse, i);
					break;
				}

				// Not added before any existing, append to end 
				if (i == CurrentTeamMiceToProcess.Num() - 1)
					CurrentTeamMiceToProcess.Add(TeamMouse);

			}
		}

		MiceToProcessMovement.Append(CurrentTeamMiceToProcess);
	}

	// Start processing, nullptr will ignore cleanup
	OnMouseProcessed(nullptr);
}


void AMM_GridManager::OnMouseProcessed(AMM_Mouse* _Mouse)
{
	// Cleanup processed mouse
	if (_Mouse)
	{
		MiceToProcessMovement.Remove(_Mouse);
		_Mouse->MouseMovementEndDelegate.RemoveDynamic(this, &AMM_GridManager::OnMouseProcessed);
		if (_Mouse->isMouseComplete())
		{
			if (MMGameMode)
			{
				// If mouse complete was winning mouse, stop processing mice
				if (MMGameMode->MouseCompleted(_Mouse))
					return;
			}
		}
	}

	// Check if remaining mice to process
	if (!MiceToProcessMovement.IsValidIndex(0))
	{
		// Reached end, all mice moved
		// TODO: Looses direct link to player that was taking the turn,
		// if the turn is somehow switched while this is processing, it will end the wrong players turn
		// could store a pointer to the player it was processing as a safety, which would then be ignored by
		// the gamemode
		if (MMGameMode)
		{
			UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::ProcessedMouse | Turn ended for current player %i as %s, all mice processed"), MMGameMode->GetCurrentPlayer()->GetCurrentTeam(), *MMGameMode->GetCurrentPlayer()->GetName());

			MMGameMode->PlayerTurnComplete(MMGameMode->GetCurrentPlayer());
		}
		else
			UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GridManager::ProcessedMouse | Failed to get Gamemode when completing mice process!"));


		return;
	}

	// Process Next mouse
	ProcessMouse(MiceToProcessMovement[0]);
}

void AMM_GridManager::ProcessMouse(AMM_Mouse* _NextMouse)
{
	if (!_NextMouse)
	{
		// TODO: ADD LOOP/Next array element
		UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GridManager::ProcessedMouse | Starting mouse not valid for processing!"));
		// Temporary stop
		return;
	}

	_NextMouse->MouseMovementEndDelegate.AddDynamic(this, &AMM_GridManager::OnMouseProcessed);

	int iTeam = _NextMouse->GetTeam();

	// Get Path
	TArray<FIntVector2D> ValidPath = GridObject->GetValidPath(_NextMouse->GetCoordinates(), iTeam == 0 ? 1 : -1);
	FIntVector2D FinalPosition = ValidPath.Last();

	// If no new position/Path, go to next mouse
	if (_NextMouse->GetCoordinates() == FinalPosition)
	{
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::ProcessedMouse | Mouse staying at %s"), *FinalPosition.ToString());
		OnMouseProcessed(_NextMouse);
		return;
	}

#if !UE_BUILD_SHIPPING
	DebugPath(ValidPath);
#endif

	// Need to take into account mice that haven't moved in path
	// Also if a second team movement then opens movement for the previously moved team?
	// Make movement
	_NextMouse->BN_StartMovement(PathFromCoordToWorld(ValidPath));
	GridObject->SetGridElement(_NextMouse->GetCoordinates(), nullptr);
	RemoveMouseFromColumn(_NextMouse->GetCoordinates().X, _NextMouse);


	// If the mouse is at the end
	if ((FinalPosition.X <= 0 && iTeam == 1) || (FinalPosition.X >= GridSize.X - 1 && iTeam == 0))
	{
		// Mouse Completed
		MouseCompleted(_NextMouse, iTeam);

	}
	// Not at the end, set coordinates to end path position
	else
	{
		GridObject->SetGridElement(FinalPosition, _NextMouse);
		AddMouseToColumn(FinalPosition.X, _NextMouse);
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::ProcessedMouse | Mouse New position for team %i at %s"), iTeam, *FinalPosition.ToString());
	}

	// Update FreeSlots array
	GridObject->RegenerateFreeSlots();
}

void AMM_GridManager::MouseCompleted(AMM_Mouse* _NextMouse, int iTeam)
{
	if (!_NextMouse)
	{
		return;
	}

	_NextMouse->MouseComplete();
	TeamMice[iTeam].Remove(_NextMouse);
	Mice.Remove(_NextMouse);
	UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::ProcessedMouse | Mouse complete for team %i at %s"), iTeam, *_NextMouse->GetCoordinates().ToString());

	// Check stalemate
	if (MMGameMode)
	{
		MMGameMode->CheckStalemateMice();
	}
}

void AMM_GridManager::DebugPath(TArray<FIntVector2D> ValidPath)
{
	// TODO: DEBUG
	auto colour = FLinearColor::MakeRandomColor();
	for (int i = 0; i < ValidPath.Num(); i++)
	{
		FVector BoxLocation = GetWorldTransformFromCoord(ValidPath[i]).GetLocation() + FVector(0, 0, 50);
		FVector BoxBounds = FVector(40 * ((float)i / (float)10) + 5);

		UKismetSystemLibrary::DrawDebugBox(GetWorld(), BoxLocation, BoxBounds, colour, FRotator::ZeroRotator, 5, 3);
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::ProcessedMouse | Current path %i: %s"), i, *ValidPath[i].ToString());
	}
}

void AMM_GridManager::RemoveMouseFromColumn(int _Column, AMM_Mouse* _Mouse)
{
	MouseColumns[_Column].Remove(_Mouse);
	
	// Check the column for this mouse's team
	int TeamToCheck = _Mouse->GetTeam();
	// Remove from available list initially
	AvailableColumnTeams[_Column].Remove(TeamToCheck);

	// Check through all mice in this column
	for (AMM_Mouse* ColumnMouse : MouseColumns[_Column])
	{
		// If a mouse in this column is the team to check
		if (ColumnMouse->GetTeam() == TeamToCheck)
		{
			// Add team to available column list for this column
			AvailableColumnTeams[_Column].Add(TeamToCheck);
			break;
		}
	}
	
}

void AMM_GridManager::AddMouseToColumn(int _Column, AMM_Mouse* _Mouse)
{
	MouseColumns[_Column].Add(_Mouse);

	// Check the column for this mouse's team
	int TeamToCheck = _Mouse->GetTeam();

	// Add team id if not already in array for this column
	AvailableColumnTeams[_Column].AddUnique(TeamToCheck);
}

void AMM_GridManager::AdjustColumn(int _Column, int _Direction)
{
	LastMovedColumn = _Column;
	
	FIntVector2D LastColumnCoord = { _Column, 0 };
	if (_Direction > 0)
		LastColumnCoord = { _Column, GridSize.Y - 1 };

	if (!IsValid(GridObject))
	{
		UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GridManager::AdjustColumn | Grid Manager not valid!"));
		return;
	}

	// Find Last element and remove from grid
	AMM_GridElement* LastElement = GridObject->GetGridElement(LastColumnCoord);
	GridObject->SetGridElement(LastColumnCoord, nullptr);

	// TODO: Change from iteration to displacing array??
	// At least minimum store in new array, then apply it and update elements

	// Slot the last element into the first slot in the loop
	AMM_GridElement* NextElement = LastElement;

	for (int i = 0; i < GridSize.Y; i++)
	{
		// y based on direction
		// if direction is negative/down, will start at the top and go down
		// Otherwise will start at 0 and go up
		int y = i;
		if (_Direction < 0)
			y = GridSize.Y - 1 - i;

		// Store current element
		FIntVector2D CurrentSlot = { _Column, y };
		AMM_GridElement* CurrentElement = GridObject->GetGridElement(CurrentSlot);

		// Set current slot to next element stored previously
		GridObject->SetGridElement(CurrentSlot, NextElement);

#if !UE_BUILD_SHIPPING
		FString NextElementDisplay = "none";
		if (NextElement)
			NextElementDisplay = NextElement->GetName();
		FString CurrentElementDisplay = "none";
		if (CurrentElement)
			CurrentElementDisplay = CurrentElement->GetName();
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::AdjustColumn | Setting element %s at %s which was previously %s"), *NextElementDisplay, *CurrentSlot.ToString(), *CurrentElementDisplay);
#endif

		// Save current element for next
		NextElement = CurrentElement;

		// First element set to last
		if (i == 0)
		{
			// TODO: Animate/visual
			// Update world position
			if (LastElement)
			{
				FVector NewLocation = GetWorldTransformFromCoord(CurrentSlot).GetLocation();
				if (ColumnControls.Contains(_Column))
				{
					NewLocation.Z += ColumnControls[_Column]->GetActorLocation().Z - GetActorLocation().Z;
				}
				LastElement->SetActorLocation(NewLocation);

				UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::AdjustColumn | Moving last element %s to %s"), *LastElement->GetName(), *CurrentSlot.ToString());
			}
		}

	}

	// Update FreeSlots array
	GridObject->RegenerateFreeSlots();

	BeginProcessMice();
}

bool AMM_GridManager::IsTeamInColumn(int _Column, int _Team)
{
	if (!AvailableColumnTeams.Contains(_Column))
		return false;

	return AvailableColumnTeams[_Column].Contains(_Team);
}

TArray<int> AMM_GridManager::GetTeamColumns(int _Team)
{
	TArray<int> AvailableColumns;

	int FailsafeColumn = -1;

	// For each column
	for (int x = 0; x < GridSize.X; x++)
	{
		// If team is available in this column, add it
		if (AvailableColumnTeams[x].Contains(_Team))
		{
			// Store failsafe column, in case all are removed
			FailsafeColumn = x;

			// Cannot move the last moved column
			if (x == LastMovedColumn)
			{
				continue;
			}

			AvailableColumns.Add(x);
		}
	}

	// If no columns were added, use failsafe column
	if (AvailableColumns.Num() <= 0)
	{
		AvailableColumns.Add(FailsafeColumn);
	}

	return AvailableColumns;
}

bool AMM_GridManager::IsStalemate() const
{
	TArray<int> MiceTeams;
	TeamMice.GenerateKeyArray(MiceTeams);
	for (int iTeam : MiceTeams)
	{
		// If any team doesn't have one mice, its not a "stalemate"
		if (TeamMice[iTeam].Num() != 1)
			return false;
	}

	return true;
}


// ################################ Grid Debugging ################################

void AMM_GridManager::SetDebugVisualGrid(bool _Enabled)
{
	bDebugGridEnabled = _Enabled;
}

void AMM_GridManager::ToggleDebugVisualGrid()
{
	SetDebugVisualGrid(!bDebugGridEnabled);
}

void AMM_GridManager::DebugVisualiseGrid()
{
	// DEBUG FOR GRID VALUES
	// For each column
	for (int x = 0; x < GridSize.X; x++)
	{

		// For each row, add to column array
		for (int y = 0; y < GridSize.Y; y++)
		{
			AMM_GridElement* gridElement = GridObject->GetGridElement({ x, y });
			auto color = FLinearColor::White;
			if (gridElement)
			{
				color = FLinearColor::Yellow;
				if (gridElement->IsA(AMM_Mouse::StaticClass()))
					color = FLinearColor::Green;
			}
			UKismetSystemLibrary::DrawDebugSphere(GetWorld(), GetWorldTransformFromCoord({ x, y }).GetLocation() + FVector(-100, 0, 50), 25.0f, 6, color, 0.5, 3);
		}
	}
}