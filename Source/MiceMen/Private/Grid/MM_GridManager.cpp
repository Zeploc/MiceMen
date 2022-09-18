// Copyright Alex Coultas, Mice Men Example Project


#include "Grid/MM_GridManager.h"

#include "Kismet/GameplayStatics.h"

#include "Grid/MM_GridElement.h"
#include "Grid/MM_GridBlock.h"
#include "Gameplay/MM_Mouse.h"
#include "Gameplay/MM_ColumnControl.h"
#include "Grid/MM_GridObject.h"
#include "MiceMen.h"
#include "Base/MM_GameMode.h"
#include "Player/MM_PlayerController.h"
#include "Player/MM_GameViewPawn.h"

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

void AMM_GridManager::BeginPlay()
{
	Super::BeginPlay();
}

void AMM_GridManager::EndPlay(EEndPlayReason::Type _EndPlayReason)
{
	Super::EndPlay(_EndPlayReason);

	GridCleanUp();
}

void AMM_GridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bDebugGridEnabled)
	{
		DisplayDebugVisualiseGrid();
	}
}

void AMM_GridManager::SetupGrid(FIntVector2D _GridSize, AMM_GameMode* _MMGameMode)
{
	GridSize = _GridSize;
	MMGameMode = _MMGameMode;
	UE_LOG(LogTemp, Display, TEXT("AMM_GridManager::SetupGrid | Setup grid with size %s and gamemode %s"), *GridSize.ToString(), *MMGameMode->GetName());
}

void AMM_GridManager::RebuildGrid(int _InitialMiceCount)
{
	// Empty old grid and remove
	GridCleanUp();

	// Check grid size is valid
	if (GridSize.X < 2 || GridSize.Y < 2)
	{
		UE_LOG(MiceMenEventLog, Error, TEXT("GRID TOO SMALL, CANNOT SETUP"));
		return;
	}

	// Create new grid
	CreateGrid();

	// Populate grid elements
	PopulateGrid();
	PopulateMice(_InitialMiceCount);
}

void AMM_GridManager::CreateGrid()
{
	// Create new grid object and setup
	GridObject = NewObject<UMM_GridObject>(this, UMM_GridObject::StaticClass());
	GridObject->SetupGrid(GridSize);

	// Remainder of division by 2, either 0 or 1
	GapSize = GridSize.X % 2;
	// Team size is the grid width without the gap, halved
	TeamSize = (GridSize.X - GapSize) / 2;
}

void AMM_GridManager::GridCleanUp()
{
	// Remove all columns
	TArray< AMM_ColumnControl*> Columns;
	ColumnControls.GenerateValueArray(Columns);
	for (AMM_ColumnControl* Column : Columns)
	{
		if (Column)
		{
			Column->Destroy();
		}
	}
	ColumnControls.Empty();

	// Remove all mice
	for (AMM_Mouse* Mouse : Mice)
	{
		if (Mouse)
		{
			Mouse->Destroy();
		}
	}
	Mice.Empty();

	// Empty Remaining containers
	MiceTeams.Empty();
	MiceColumns.Empty();
	OccupiedTeamsPerColumn.Empty();
	LastMovedColumn = -1;
	MiceToProcessMovement.Empty();

	// Remaining grid cleanup
	if (GridObject)
	{
		GridObject->CleanUp();
		GridObject->ConditionalBeginDestroy();
		GridObject = nullptr;
	}
}

void AMM_GridManager::PopulateGrid()
{
	UE_LOG(LogTemp, Display, TEXT("AMM_GridManager::PopulateGrid | Populating grid of size %s"), *GridSize.ToString());

	// For each column
	for (int x = 0; x < GridSize.X; x++)
	{
		// Check class is valid
		if (!ColumnControlClass)
		{
			ColumnControlClass = AMM_ColumnControl::StaticClass();
		}

		// Setup new column
		FTransform ColumnTransform = GetWorldTransformFromCoord(FIntVector2D(x, 0));
		AMM_ColumnControl* NewColumnControl = GetWorld()->SpawnActorDeferred<AMM_ColumnControl>(ColumnControlClass, ColumnTransform);
		NewColumnControl->SetupColumn(x, this);
		UGameplayStatics::FinishSpawningActor(NewColumnControl, ColumnTransform);
		ColumnControls.Add(x, NewColumnControl);

		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::PopulateGrid | Adding collumn at %i"), x);

		// Store new column in for game play use
		MiceColumns.Add(x, TArray<AMM_Mouse*>());
		OccupiedTeamsPerColumn.Add(x, TArray<int>());

		// For each row, add to column array
		for (int y = 0; y < GridSize.Y; y++)
		{
			// Place random block (or center blocks) or an empty slot
			PopulateGridElement({ x, y }, NewColumnControl);
		}
	}
}

void AMM_GridManager::PopulateGridElement(FIntVector2D _NewCoord, AMM_ColumnControl* NewColumnControl)
{
	// Initial variables
	FTransform GridElementTransform = GetWorldTransformFromCoord(_NewCoord);

	// Prepare center pieces, creates blocking center where mice can't cross at the start
	// If gap size is 1, alternating blocks will be either side
	// If gap size is 0, will have one side place a block every third y pos, and the other side the opposite
	bool bIsPresetCenterBlock = _NewCoord.X >= TeamSize - 1 && _NewCoord.X <= TeamSize + GapSize;
	
	// Default to not have preset block
	bool bisPresetBlockFilled = false;
	if (bIsPresetCenterBlock)
	{
		// Center column
		if (_NewCoord.X == TeamSize)
		{
			// If its not every third block, vertical position not dividable by 3
			bisPresetBlockFilled = _NewCoord.Y % 3 != 0;
		}
		else
		{
			// If every third block, vertical position dividable by 3
			bisPresetBlockFilled = _NewCoord.Y % 3 == 0;
		}
	}

	// Initial testing random bool for placement unless overridden by center blocks
	if ((FMath::RandBool() && !bIsPresetCenterBlock) || bisPresetBlockFilled)
	{
		// Create new Grid block object
		AMM_GridBlock* NewGridBlock = GetWorld()->SpawnActor<AMM_GridBlock>(GridBlockClass, GridElementTransform);
		if (!NewGridBlock)
		{
			UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GridManager::PlaceBlock | Failed to create Grid Block!"));
			return;
		}
		NewGridBlock->SetupGridInfo(this, _NewCoord);

		// Add to column array and attach
		if (NewColumnControl)
		{
			NewGridBlock->AttachToActor(NewColumnControl, FAttachmentTransformRules::KeepWorldTransform);
		}
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
	// Define team initial position ranges
	FIntVector2D TeamRanges[] =
	{
		// Left size, to the size of team size
		FIntVector2D(0, TeamSize - 1),
		// Right side, starting to the right of the center blocks to the end of the grid
		FIntVector2D(TeamSize + GapSize, GridSize.X - 1)
	};

	UE_LOG(LogTemp, Display, TEXT("AMM_GridManager::PopulateMice | Populating mice with team ranges %s and %s"), *TeamRanges[0].ToString(), *TeamRanges[1].ToString());

	// Place mice for each team,
	// Note: Possibility for more than 2 teams
	for (int iTeam = 0; iTeam < 2; iTeam++)
	{
		// Add initial team array and store in game mode
		MiceTeams.Add(iTeam, TArray<AMM_Mouse*>());
		if (MMGameMode)
		{
			MMGameMode->AddTeam(iTeam);
		}
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::PopulateMice | Adding mice for team %i"), iTeam);

		// Add mice for current team, based on the passed in number of mice
		for (int iMouse = 0; iMouse < _MicePerTeam; iMouse++)
		{
			// Get initial position for mice based on free slots with the team range
			FIntVector2D NewRandomMousePosition = GridObject->GetRandomGridCoordInColumnRange(TeamRanges[iTeam].X, TeamRanges[iTeam].Y);

			// Get final position for mice (auto moves on initial placement)
			TArray<FIntVector2D> ValidPath = GridObject->GetValidPath(NewRandomMousePosition, iTeam == 0 ? 1 : -1);
			NewRandomMousePosition = ValidPath.Last();

			// Get coordinates in world
			FTransform GridElementTransform = GetWorldTransformFromCoord(NewRandomMousePosition);

			// Setup new Mouse
			AMM_Mouse* NewMouse = GetWorld()->SpawnActorDeferred<AMM_Mouse>(MouseClass, GridElementTransform);
			NewMouse->SetupGridInfo(this, NewRandomMousePosition);
			NewMouse->SetupMouse(iTeam);
			UGameplayStatics::FinishSpawningActor(NewMouse, GridElementTransform);

			// Attach to column and store
			NewMouse->AttachToActor(ColumnControls[NewRandomMousePosition.X], FAttachmentTransformRules::KeepWorldTransform);
			AddMouseToColumn(NewRandomMousePosition.X, NewMouse);

			// Store mice in grid and team
			GridObject->SetGridElement(NewRandomMousePosition, NewMouse);
			Mice.Add(NewMouse);
			MiceTeams[iTeam].Add(NewMouse);

			UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::PopulateMice | Adding mice for team %i at %s"), iTeam, *NewRandomMousePosition.ToString());
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
	// Save all mice in the correct order based on the current team as the starting mice
	StoreOrderedMiceToProcess();

#if !UE_BUILD_SHIPPING
	if (MMGameMode->GetCurrentGameType() == EGameType::E_TEST)
	{
		// If running a test, stop processing if not all mice were processed
		if (!DebugCheckAllMiceProcessed())
		{
			return;
		}
	}
#endif

	// Start processing, nullptr will ignore cleanup
	ProcessCompletedMouseMovement(nullptr);
}

void AMM_GridManager::StoreOrderedMiceToProcess()
{
	TArray<int> OrderedTeamsToProcess;

	if (MMGameMode && MMGameMode->GetCurrentPlayer())
	{
		// Order the process of mice, starting with the current player's team
		int CurrentPlayerTeam = MMGameMode->GetCurrentPlayer()->GetCurrentTeam();
		OrderedTeamsToProcess.Add(CurrentPlayerTeam);
		OrderedTeamsToProcess.Add(1 - CurrentPlayerTeam);
	}

	for (int iTeam : OrderedTeamsToProcess)
	{
		// Mice for this team not added, cannot process
		if (!MiceTeams.Contains(iTeam))
		{
			UE_LOG(MiceMenEventLog, Warning, TEXT("AMM_GridManager::StoreOrderedMiceToProcess | Unable to process mice for team %i, missing from Team Mice map"), iTeam);
			continue;
		}

		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::StoreOrderedMiceToProcess | Processing mice for team %i"), iTeam);

		// Order needs to be more "forward/lower" players going first
		// ie FIRST lower players go first, so that higher players fall and can move after
		// THEN forward players (closer towards their end) go first so they don't block other mice

		TArray<AMM_Mouse*> CurrentTeamMiceToProcess;

		// TODO: Improve Logic
		// NOTE: Storing the Mice and checking/inserting in order, is more efficient than iterating through the whole grid
		for (AMM_Mouse* TeamMouse : MiceTeams[iTeam])
		{
			// Add first one without iteration
			if (CurrentTeamMiceToProcess.Num() <= 0)
			{
				CurrentTeamMiceToProcess.Add(TeamMouse);
				continue;
			}

			// Check through existing ordered mice
			FIntVector2D Coordinates = TeamMouse->GetCoordinates();
			for (int i = 0; i < CurrentTeamMiceToProcess.Num(); i++)
			{
				// If new mouse is lower than or on the same line current being checked against
				AMM_Mouse* OrderedMouse = CurrentTeamMiceToProcess[i];
				if (Coordinates.Y <= OrderedMouse->GetCoordinates().Y)
				{
					// IF its on the same row, will check the new mouse is more forward
					if (Coordinates.Y == OrderedMouse->GetCoordinates().Y)
					{
						// Check if the mouse is more forward, direction based on the team
						bool bIsMoreForward = Coordinates.X > OrderedMouse->GetCoordinates().X;

						// Team is going left
						if (iTeam == 1)
						{
							bIsMoreForward = Coordinates.X < OrderedMouse->GetCoordinates().X;
						}

						// Mouse is not more forward, go to next
						if (!bIsMoreForward)
							continue;
					}

					// Insert at current
					CurrentTeamMiceToProcess.Insert(TeamMouse, i);
					break;
				}
			}
			// Add to end (if not already) to ensure they are added to process
			CurrentTeamMiceToProcess.AddUnique(TeamMouse);

			//TODO: Breaks processing of mice
			//// Set up delegate for when movement is complete
			//TeamMouse->MouseMovementEndDelegate.AddDynamic(this, &AMM_GridManager::ProcessCompletedMouseMovement);
		}

		MiceToProcessMovement.Append(CurrentTeamMiceToProcess);
	}
}

void AMM_GridManager::ProcessCompletedMouseMovement(AMM_Mouse* _Mouse)
{
	// Cleanup processed mouse
	CleanupProcessedMouse(_Mouse);

	// Check mouse has reached the end
	if (_Mouse && _Mouse->HasReachedEnd())
	{
		if (!MMGameMode)
		{
			UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GridManager::ProcessCompletedMouseMovement | Gamemode invalid when checking if mouse has reached its goal!"));
			return;
		}

		int MouseTeam = _Mouse->GetTeam();

		MMGameMode->AddScore(MouseTeam);

		if (MMGameMode->HasTeamWon(MouseTeam))
		{
			// Mouse completing and was winning mouse, stop processing mice
			return;
		}
	}

	bool bAnyRemainingMiceToProcess = MiceToProcessMovement.IsValidIndex(0);

	// There are still mice to process
	if (bAnyRemainingMiceToProcess)
	{
		// Process Next mouse
		ProcessMouse(MiceToProcessMovement[0]);
	}
	else
	{
		ProcessMiceComplete();
	}
}

void AMM_GridManager::ProcessMiceComplete()
{
	// Reached end, all mice processed
	// TODO: Loses direct link to player that was taking the turn,
	// if the turn is somehow switched while this is processing, it will end the wrong players turn
	// could store a pointer to the player it was processing as a safety, which would then be ignored by
	// the game mode if incorrect
	if (MMGameMode)
	{
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::ProcessMiceComplete | Turn ended for current player %i as %s, all mice processed"), MMGameMode->GetCurrentPlayer()->GetCurrentTeam(), *MMGameMode->GetCurrentPlayer()->GetName());

		MMGameMode->PlayerTurnComplete(MMGameMode->GetCurrentPlayer());
	}
	else
	{
		UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GridManager::ProcessMiceComplete | Failed to get Gamemode when completing mice process!"));
	}
}

void AMM_GridManager::CleanupProcessedMouse(AMM_Mouse* _Mouse)
{
	if (_Mouse)
	{
		MiceToProcessMovement.Remove(_Mouse);
		_Mouse->MovementEndDelegate.RemoveDynamic(this, &AMM_GridManager::ProcessCompletedMouseMovement);			
	}
}

void AMM_GridManager::ProcessMouse(AMM_Mouse* _Mouse)
{
	// Check if mouse valid, otherwise go on to next
	if (!_Mouse)
	{
		UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GridManager::ProcessMouse | Mouse not valid for processing!"));

		// Remove null ptr from array
		if (MiceToProcessMovement.IsValidIndex(0) && MiceToProcessMovement[0] == _Mouse)
		{
			MiceToProcessMovement.RemoveAt(0);
		}

		ProcessCompletedMouseMovement(_Mouse);
		return;
	}

	// Set up delegate for when movement is complete
	_Mouse->MovementEndDelegate.AddDynamic(this, &AMM_GridManager::ProcessCompletedMouseMovement);

	// Move Mouse to next position
	FIntVector2D FinalPosition;
	bool bSuccessfulMovement = MoveMouse(_Mouse, FinalPosition);
	if (!bSuccessfulMovement)
	{
		// Mouse didn't move, go on to the next mouse
		ProcessCompletedMouseMovement(_Mouse);
		return;
	}

	// Remove mouse from previous location
	GridObject->SetGridElement(_Mouse->GetCoordinates(), nullptr);
	RemoveMouseFromColumn(_Mouse->GetCoordinates().X, _Mouse);

	int iTeam = _Mouse->GetTeam();

	// If the mouse is at the end
	if ((FinalPosition.X <= 0 && iTeam == 1) || (FinalPosition.X >= GridSize.X - 1 && iTeam == 0))
	{
		// Mouse Completed
		MouseGoalReached(_Mouse, iTeam);
	}
	// Not at the end, set coordinates to end path position
	else
	{
		GridObject->SetGridElement(FinalPosition, _Mouse);
		AddMouseToColumn(FinalPosition.X, _Mouse);
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::ProcessMouse | Mouse New position for team %i at %s"), iTeam, *FinalPosition.ToString());
	}

	// If test mode, move delegate is not fired on mouse, go straight to on processed
	if (MMGameMode->GetCurrentGameType() == EGameType::E_TEST)
	{
		ProcessCompletedMouseMovement(_Mouse);
	}
}

bool AMM_GridManager::MoveMouse(AMM_Mouse* _NextMouse, FIntVector2D& _FinalPosition)
{
	// Calculate Path
	int Direction = _NextMouse->GetTeam() == 0 ? 1 : -1;
	TArray<FIntVector2D> ValidPath = GridObject->GetValidPath(_NextMouse->GetCoordinates(), Direction);
	_FinalPosition = ValidPath.Last();

	// If there is no new position/Path, go to next mouse
	if (_NextMouse->GetCoordinates() == _FinalPosition)
	{
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::MoveMouse | Mouse staying at %s"), *_FinalPosition.ToString());
		return false;
	}

#if !UE_BUILD_SHIPPING
	DebugPath(ValidPath);
#endif

	// TODO: Need to account for a second team movement then opens movement for the previously moved team?

	// If test mode, instantly move mouse
	if (MMGameMode->GetCurrentGameType() == EGameType::E_TEST)
	{
		_NextMouse->SetActorLocation(GetWorldTransformFromCoord(_FinalPosition).GetLocation());
	}
	else
	{
		// Begin movement (should fire delegate on complete)
		_NextMouse->BN_StartMovement(PathFromCoordToWorld(ValidPath));
	}

	return true;
}

void AMM_GridManager::MouseGoalReached(AMM_Mouse* _NextMouse, int _iTeam)
{
	// Check valid
	if (!_NextMouse)
	{
		return;
	}

	// Events for mouse on goal reached
	_NextMouse->GoalReached();

	// Cleanup from grid
	MiceTeams[_iTeam].Remove(_NextMouse);
	Mice.Remove(_NextMouse);
	UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::MouseGoalReached | Mouse reached goal for team %i at %s"), _iTeam, *_NextMouse->GetCoordinates().ToString());

	// Check stalemate
	if (MMGameMode)
	{
		MMGameMode->CheckStalemateMice();
	}
}

void AMM_GridManager::RemoveMouseFromColumn(int _Column, AMM_Mouse* _Mouse)
{
	MiceColumns[_Column].Remove(_Mouse);
	
	// Check the column for this mouse's team
	int TeamToCheck = _Mouse->GetTeam();
	// Remove from available list initially
	OccupiedTeamsPerColumn[_Column].Remove(TeamToCheck);

	// Check through all mice in this column
	for (AMM_Mouse* ColumnMouse : MiceColumns[_Column])
	{
		// If a mouse in this column is the team to check
		if (ColumnMouse->GetTeam() == TeamToCheck)
		{
			// Add team to available column list for this column
			OccupiedTeamsPerColumn[_Column].Add(TeamToCheck);
			break;
		}
	}	
}

void AMM_GridManager::AddMouseToColumn(int _Column, AMM_Mouse* _Mouse)
{
	MiceColumns[_Column].Add(_Mouse);

	// Check the column for this mouse's team
	int TeamToCheck = _Mouse->GetTeam();

	// Add team id if not already in array for this column
	OccupiedTeamsPerColumn[_Column].AddUnique(TeamToCheck);
}

void AMM_GridManager::AdjustColumn(int _Column, int _Direction)
{
	// If direction is 0, no change will occur
	if (_Direction == 0)
	{
		return;
	}

	LastMovedColumn = _Column;

	// Check grid object valid
	if (!IsValid(GridObject))
	{
		UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GridManager::AdjustColumn | Grid Manager not valid!"));
		return;
	}

	// Move all elements in column in a direction (up or down) by one unit
	AMM_GridElement* LastElement = GridObject->MoveColumnElements(_Column, _Direction);

	// Move Last element to correct position in world position
	if (LastElement)
	{
		FIntVector2D CurrentSlot = LastElement->GetCoordinates();
		FVector NewLocation = GetWorldTransformFromCoord(CurrentSlot).GetLocation();
		// Displace based on column's current offset to set the correct relative location
		if (ColumnControls.Contains(_Column))
		{
			NewLocation.Z += ColumnControls[_Column]->GetActorLocation().Z - GetActorLocation().Z;
		}

		// TODO: Animate/visual
		LastElement->SetActorLocation(NewLocation);

		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::AdjustColumn | Moving last element %s to %s"), *LastElement->GetName(), *CurrentSlot.ToString());
	}

	// Start processing mice based on column change
	BeginProcessMice();
}

bool AMM_GridManager::IsTeamInColumn(int _Column, int _Team)
{
	if (!OccupiedTeamsPerColumn.Contains(_Column))
	{
		return false;
	}

	return OccupiedTeamsPerColumn[_Column].Contains(_Team);
}

TArray<int> AMM_GridManager::GetTeamColumns(int _Team)
{
	TArray<int> AvailableColumns;

	int FailsafeColumn = -1;

	// For each column
	for (int x = 0; x < GridSize.X; x++)
	{
		// If team is available in this column, add it
		if (OccupiedTeamsPerColumn[x].Contains(_Team))
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

	// If no columns were added, use failsafe column (if was set)
	if (AvailableColumns.Num() <= 0 && FailsafeColumn >= 0)
	{
		AvailableColumns.Add(FailsafeColumn);
	}

	return AvailableColumns;
}

bool AMM_GridManager::IsStalemate() const
{
	TArray<int> MiceTeamsArray;
	MiceTeams.GenerateKeyArray(MiceTeamsArray);
	for (int iTeam : MiceTeamsArray)
	{
		// If any team doesn't have one mice, its not a "stalemate"
		if (MiceTeams[iTeam].Num() != 1)
		{
			return false;
		}
	}

	return true;
}

int AMM_GridManager::GetWinningStalemateTeam() const
{
	TArray<int> TeamDistances;
	TArray<int> MiceTeamsArray;
	MiceTeams.GenerateKeyArray(MiceTeamsArray);
	TeamDistances.SetNum(MiceTeams.Num());
	for (int iTeam : MiceTeamsArray)
	{
		if (MiceTeams[iTeam].IsValidIndex(0))
		{
			AMM_Mouse* RemainingMouse = MiceTeams[iTeam][0];
			// Get the distance from the team starting point
			int Distance = FMath::Abs(((GridSize.X - 1) * iTeam) - RemainingMouse->GetCoordinates().X);

			TeamDistances[iTeam] = Distance;
		}
	}
	
	// Check both team distances were stored
	if (TeamDistances.Num() >= 2)
	{
		// Check if same distance, tie situation
		if (TeamDistances[0] == TeamDistances[1])
		{
			// -1 meaning tie, ie no winner
			return -1;
		}

		// Check winning team based on further distance
		int WinningTeam = 0;
		if (TeamDistances[1] > TeamDistances[0])
		{
			WinningTeam = 1;
		}
		return WinningTeam;
	}
	

	UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GridManager::GetWinningStalemateTeam | Failed to get winning stalemate team, returned -2!"));

	return -2;

}

// ################################ Grid Debugging ################################

void AMM_GridManager::DebugPath(TArray<FIntVector2D> ValidPath)
{
	auto colour = FLinearColor::MakeRandomColor();
	for (int i = 0; i < ValidPath.Num(); i++)
	{
		FVector BoxLocation = GetWorldTransformFromCoord(ValidPath[i]).GetLocation();
		BoxLocation.Z += GridElementHeight / 2.0;
		FVector BoxBounds = FVector(40 * ((float)i / (float)10) + 5);

		UKismetSystemLibrary::DrawDebugBox(GetWorld(), BoxLocation, BoxBounds, colour, FRotator::ZeroRotator, 5, 3);
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::ProcessedMouse | Current path %i: %s"), i, *ValidPath[i].ToString());
	}
}

void AMM_GridManager::SetDebugVisualGrid(bool _bEnabled)
{
	bDebugGridEnabled = _bEnabled;
}

void AMM_GridManager::ToggleDebugVisualGrid()
{
	SetDebugVisualGrid(!bDebugGridEnabled);
}

bool AMM_GridManager::DebugCheckAllMiceProcessed() const
{
	for (int iTeam = 0; iTeam < 2; iTeam++)
	{
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::BeginProcessMice | Checking ordered mice team %i"), iTeam);
		for (AMM_Mouse* Mouse : MiceTeams[iTeam])
		{
			if (Mouse && !MiceToProcessMovement.Contains(Mouse))
			{
				UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GridManager::BeginProcessMice | MISSING Mouse at position %s"), *Mouse->GetCoordinates().ToString());
				return false;
			}
		}
	}
	return true;
}

void AMM_GridManager::DisplayDebugVisualiseGrid()
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