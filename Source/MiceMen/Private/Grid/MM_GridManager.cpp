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

	GridBlockClass = AMM_GridBlock::StaticClass();
	MouseClass = AMM_Mouse::StaticClass();
	ColumnControlClass = AMM_ColumnControl::StaticClass();
}

void AMM_GridManager::BeginPlay()
{
	Super::BeginPlay();
}

void AMM_GridManager::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GridCleanUp();
}

void AMM_GridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if !UE_BUILD_SHIPPING
	if (bDisplayDebugGrid)
	{
		DisplayDebugGrid();
	}
#endif
}

void AMM_GridManager::SetupGridVariables(const FIntVector2D& InGridSize, AMM_GameMode* InMMGameMode)
{
	GridSize = InGridSize;
	MMGameMode = InMMGameMode;
	UE_LOG(LogTemp, Display, TEXT("AMM_GridManager::SetupGridVariables | Setup grid with size %s and gamemode %s"),
	       *GridSize.ToString(), MMGameMode ? *MMGameMode->GetName() : TEXT("none"));
}

void AMM_GridManager::RebuildGrid(const int InitialMiceCount)
{
	// Empty old grid and clear containers
	GridCleanUp();

	// Check grid size is valid
	if (GridSize.X < 2 || GridSize.Y < 2)
	{
		UE_LOG(MiceMenEventLog, Error, TEXT("GRID TOO SMALL, CANNOT SETUP"));
		return;
	}

	// Create new grid and setup sizes
	CreateGrid();

	// Populate grid elements
	PopulateGrid();
	PopulateTeams(InitialMiceCount);
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
	for (const TPair<int, AMM_ColumnControl*>& Column : ColumnControls)
	{
		if (Column.Value)
		{
			Column.Value->Destroy();
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
	for (AMM_Mouse* Mouse : CompletedMice)
	{
		if (Mouse)
		{
			Mouse->Destroy();
		}
	}
	CompletedMice.Empty();

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
		FTransform ColumnTransform = CoordToWorldTransform(FIntVector2D(x, 0));
		AMM_ColumnControl* NewColumnControl = GetWorld()->SpawnActorDeferred<AMM_ColumnControl>(
			ColumnControlClass, ColumnTransform);
		NewColumnControl->SetupColumn(x, this);
		UGameplayStatics::FinishSpawningActor(NewColumnControl, ColumnTransform);
		ColumnControls.Add(x, NewColumnControl);

		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::PopulateGrid | Adding collumn at %i"), x);

		// Store new column for game play use
		MiceColumns.Add(x, TArray<AMM_Mouse*>());
		OccupiedTeamsPerColumn.Add(x, TArray<ETeam>());

		// For each row, add to column array
		for (int y = 0; y < GridSize.Y; y++)
		{
			// Place random block (or center blocks) or an empty slot
			PlaceGridElement({x, y}, NewColumnControl);
		}

		// Remove blocks based on sparseness
		int BlocksToRemove = static_cast<int>(static_cast<float>(GridSize.Y) * BlockSparseness);
		// Needs to remove a minimum of one (can't have a column fully taken up)
		BlocksToRemove = FMath::Max(BlocksToRemove, 1);
		for (int i = 0; i < BlocksToRemove; i++)
		{
			const int RandomY = FMath::RandRange(0, GridSize.Y - 1);
			FIntVector2D RandomCoord = FIntVector2D(x, RandomY);
			// Not affect center blocks
			if (IsCoordInCenterGroup(RandomCoord))
			{
				continue;
			}
			// Check grid element exists
			AMM_GridElement* CurrentGridElement = GridObject->GetGridElement(RandomCoord);
			if (CurrentGridElement)
			{
				// Remove grid element
				UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::PopulateGrid | Removing block in column %i at %i"), x, RandomY);
				GridObject->SetGridElement(RandomCoord, nullptr);
				CurrentGridElement->CleanUp();
				CurrentGridElement->Destroy();
			}
		}
	}
}

bool AMM_GridManager::IsCoordInCenterGroup(const FIntVector2D& NewCoord) const
{
	// If gap size is 1, alternating blocks will be either side
	// If gap size is 0, will have one side place a block every third y pos, and the other side the opposite
	
	const int Start = TeamSize - 1;
	const int End = TeamSize + GapSize;
	return NewCoord.X >= Start && NewCoord.X <= End;
}

void AMM_GridManager::PlaceGridElement(const FIntVector2D& NewCoord, AMM_ColumnControl* ColumnControl)
{
	// Initial variables
	const FTransform GridElementTransform = CoordToWorldTransform(NewCoord);

	// Prepare center pieces, creates blocking center where mice can't cross at the start
	const bool bIsPresetCenterBlock = IsCoordInCenterGroup(NewCoord);

	// Default to not have preset block
	bool bIsPresetBlockFilled = false;
	if (bIsPresetCenterBlock)
	{
		// If coord is in the center column
		if (NewCoord.X == TeamSize)
		{
			// If vertical position not dividable by 3, then its not every third block
			bIsPresetBlockFilled = NewCoord.Y % 3 != 0;
		}
		else
		{
			// If vertical position dividable by 3, then its every third block 
			bIsPresetBlockFilled = NewCoord.Y % 3 == 0;
		}
	}

	// Initial testing random bool for placement unless overridden by center blocks
	if ((FMath::RandBool() && !bIsPresetCenterBlock) || bIsPresetBlockFilled)
	{
		// Create new Grid block object
		AMM_GridBlock* NewGridBlock = GetWorld()->SpawnActor<AMM_GridBlock>(GridBlockClass, GridElementTransform);
		if (!NewGridBlock)
		{
			UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GridManager::PlaceGridElement | Failed to create Grid Block!"));
			return;
		}
		NewGridBlock->SetupGridVariables(this, MMGameMode, NewCoord);

		// Store element and attach
		if (ColumnControl)
		{
			NewGridBlock->AttachToActor(ColumnControl, FAttachmentTransformRules::KeepWorldTransform);
		}
		GridObject->SetGridElement(NewCoord, NewGridBlock);

		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::PlaceGridElement | Adding grid block at %s"), *NewCoord.ToString());
	}
	else
	{
		// Set element as empty
		GridObject->SetGridElement(NewCoord, nullptr);
	}
}

void AMM_GridManager::PopulateTeams(int MicePerTeam)
{
	// Define team initial position ranges
	TMap<ETeam, FIntVector2D> TeamRanges;

	// Left side range in grid, to the team size
	TeamRanges.Add(ETeam::E_TEAM_A, FIntVector2D(0, TeamSize - 1));

	// Right side range in grid, starting to the right of the center blocks to the end of the grid
	TeamRanges.Add(ETeam::E_TEAM_B, FIntVector2D(TeamSize + GapSize, GridSize.X - 1));

	UE_LOG(LogTemp, Display, TEXT("AMM_GridManager::PopulateTeams | Populating mice with team ranges %s and %s"), *TeamRanges[ETeam::E_TEAM_A].ToString(), *TeamRanges[ETeam::E_TEAM_B].ToString());

	// Place mice for each team
	for (ETeam CurrentTeam = ETeam::E_TEAM_A; CurrentTeam < ETeam::E_MAX; ++CurrentTeam)
	{
		// Add initial team array and store in game mode
		MiceTeams.Add(CurrentTeam, TArray<AMM_Mouse*>());
		if (MMGameMode)
		{
			MMGameMode->AddTeam(CurrentTeam);
		}
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::PopulateTeams | Adding mice for team %i"), CurrentTeam);

		// Add mice for current team, based on the passed in number of mice
		for (int iMouse = 0; iMouse < MicePerTeam; iMouse++)
		{
			// Get initial position for mice based on free slots with the team range
			FIntVector2D NewRandomMousePosition = GridObject->GetRandomGridCoordInColumnRange(TeamRanges[CurrentTeam].X, TeamRanges[CurrentTeam].Y);

			// Setup new Mouse
			AMM_Mouse* NewMouse = GetWorld()->SpawnActorDeferred<AMM_Mouse>(MouseClass, FTransform::Identity);
			NewMouse->SetupGridVariables(this, MMGameMode, NewRandomMousePosition);

			// Setup mouse updates position based on movement path
			NewMouse->SetupMouse(CurrentTeam, NewRandomMousePosition);

			// Get final position for mice (auto moves on initial placement) in the world
			const FTransform GridElementTransform = CoordToWorldTransform(NewRandomMousePosition);
			UGameplayStatics::FinishSpawningActor(NewMouse, GridElementTransform);

			// Attach to column and store
			NewMouse->AttachToActor(ColumnControls[NewRandomMousePosition.X], FAttachmentTransformRules::KeepWorldTransform);
			AddMouseToColumn(NewRandomMousePosition.X, NewMouse);

			// Store mice in grid and team
			GridObject->SetGridElement(NewRandomMousePosition, NewMouse);
			Mice.Add(NewMouse);
			MiceTeams[CurrentTeam].Add(NewMouse);

			UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::PopulateTeams | Adding mice for team %i at %s"), CurrentTeam, *NewRandomMousePosition.ToString());
		}
	}
}

TArray<FVector> AMM_GridManager::PathCoordToWorld(const TArray<FIntVector2D>& CoordPath) const
{
	TArray<FVector> NewWorldPath;
	for (const FIntVector2D& Coord : CoordPath)
	{
		const FVector WorldLocation = CoordToWorldTransform(Coord).GetLocation();
		NewWorldPath.Add(WorldLocation);
	}
	return NewWorldPath;
}

FTransform AMM_GridManager::CoordToWorldTransform(const FIntVector2D& Coords) const
{
	FTransform GridElementTransform = GetActorTransform();
	FVector NewRelativeLocation = FVector::Zero();

	// Since X is forward, horizontal axis is Y, and vertical axis is Z

	// Starting Bottom Left
	NewRelativeLocation.Y -= (GridSize.X / 2) * GridElementWidth;

	// Add coordinate offset based on grid element size
	NewRelativeLocation.Y += Coords.X * GridElementWidth;
	NewRelativeLocation.Z += Coords.Y * GridElementHeight;

	// Transform from relative to world
	const FVector NewWorldLocation = GetActorTransform().TransformPosition(NewRelativeLocation);

	// Apply location to new transform
	GridElementTransform.SetLocation(NewWorldLocation);

	return GridElementTransform;
}

EDirection AMM_GridManager::GetDirectionFromTeam(ETeam Team)
{
	EDirection Direction = EDirection::E_LEFT;
	if (Team == ETeam::E_TEAM_A)
	{
		Direction = EDirection::E_RIGHT;
	}
	return Direction;
}

void AMM_GridManager::BeginProcessMice()
{
	// Keeps a reference to the player to keep a link to who's turn is being processed as a safety
	// If the turn is somehow switched while this is processing, it will then be ignored by
	// the game mode once complete
	CurrentPlayerProcessing = MMGameMode->GetCurrentPlayer();

	// Save all mice in the correct order based on the current team as the starting mice
	StoreOrderedMiceToProcess();

	// Check test mode
	if (MMGameMode->GetCurrentGameType() == EGameType::E_TEST)
	{
		// If running a test, stop processing if not all mice were processed
		if (!DebugCheckAllMiceProcessed())
		{
			return;
		}
	}

	// Start processing, nullptr will ignore cleanup
	CurrentProcessedMovedMiceCount = 0;
	HandleCompletedMouseMovement(nullptr);
}

void AMM_GridManager::StoreOrderedMiceToProcess()
{
	TArray<ETeam> OrderedTeamsToProcess;

	if (MMGameMode && CurrentPlayerProcessing)
	{
		// Order the process of mice, starting with the current player's team
		const ETeam CurrentPlayerTeam = CurrentPlayerProcessing->GetCurrentTeam();
		OrderedTeamsToProcess.Add(CurrentPlayerTeam);

		// Add other team
		ETeam SecondTeam = ETeam::E_TEAM_B;
		if (CurrentPlayerTeam == ETeam::E_TEAM_B)
		{
			SecondTeam = ETeam::E_TEAM_A;
		}
		OrderedTeamsToProcess.Add(SecondTeam);
	}

	for (const ETeam CurrentTeam : OrderedTeamsToProcess)
	{
		// Mice for this team not added, cannot process
		if (!MiceTeams.Contains(CurrentTeam))
		{
			UE_LOG(MiceMenEventLog, Warning, TEXT("AMM_GridManager::StoreOrderedMiceToProcess | Unable to process mice for team %i, missing from Team Mice map"), CurrentTeam);
			continue;
		}

		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::StoreOrderedMiceToProcess | Processing mice for team %i"), CurrentTeam);

		// The order is players who are lower, and front most, going first
		// ie FIRST lower row players go, so that higher players fall and can move after
		// and THEN for each in that row, forward players (closer towards their end) so they don't block other mice

		TArray<AMM_Mouse*> CurrentTeamMiceToProcess;

		// NOTE: Storing the Mice and checking/inserting in order, is more efficient than iterating through the whole grid
		for (AMM_Mouse* TeamMouse : MiceTeams[CurrentTeam])
		{
			// Add first one without iteration
			if (CurrentTeamMiceToProcess.Num() <= 0)
			{
				CurrentTeamMiceToProcess.Add(TeamMouse);
				continue;
			}

			// Add mouse based on position
			InsertMouseByOrder(TeamMouse, CurrentTeamMiceToProcess);
		}

		MiceToProcessMovement.Append(CurrentTeamMiceToProcess);
	}
}

void AMM_GridManager::InsertMouseByOrder(AMM_Mouse* TeamMouse, TArray<AMM_Mouse*>& CurrentTeamMiceToProcess)
{
	const ETeam CurrentTeam = TeamMouse->GetTeam();

	// Check through existing ordered mice
	const FIntVector2D Coordinates = TeamMouse->GetCoordinates();
	for (int i = 0; i < CurrentTeamMiceToProcess.Num(); i++)
	{
		const AMM_Mouse* OrderedMouse = CurrentTeamMiceToProcess[i];

		// If new mouse is lower than or on the same line current being checked against
		if (Coordinates.Y <= OrderedMouse->GetCoordinates().Y)
		{
			// If its on the same row, will check the new mouse is more forward
			if (Coordinates.Y == OrderedMouse->GetCoordinates().Y)
			{
				// Check if the mouse is more forward, direction based on the team
				bool bIsMoreForward = Coordinates.X > OrderedMouse->GetCoordinates().X;

				// Team is going left
				if (CurrentTeam == ETeam::E_TEAM_B)
				{
					bIsMoreForward = Coordinates.X < OrderedMouse->GetCoordinates().X;
				}

				// Mouse is not more forward, go to next
				if (!bIsMoreForward)
				{
					continue;
				}
			}

			// Insert at current
			CurrentTeamMiceToProcess.Insert(TeamMouse, i);
			break;
		}
	}

	// Add to end (if not already) to ensure they are added to process
	CurrentTeamMiceToProcess.AddUnique(TeamMouse);
}

void AMM_GridManager::ProcessMouse(AMM_Mouse* Mouse)
{
	// Check the mouse is valid to process
	if (!CheckMouse(Mouse))
	{
		return;
	}

	// Set up delegate for when movement is complete
	Mouse->MovementEndDelegate.AddDynamic(this, &AMM_GridManager::HandleCompletedMouseMovement);

	// Attempt to move the mouse
	const bool bMovementSuccesful = Mouse->AttemptPerformMovement();
	if (!bMovementSuccesful)
	{
		return;
	}

	// A mice successfully moved
	CurrentProcessedMovedMiceCount++;

	// If test mode go straight to movement complete, as move delegate is not fired on mouse 
	if (MMGameMode->GetCurrentGameType() == EGameType::E_TEST)
	{
		HandleCompletedMouseMovement(Mouse);
	}
}

void AMM_GridManager::HandleCompletedMouseMovement(AMM_Mouse* Mouse)
{
	// Cleanup processed mouse
	CleanupProcessedMouse(Mouse);

	// Check mouse has reached the end
	if (Mouse && Mouse->HasReachedEnd())
	{
		if (!MMGameMode)
		{
			UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GridManager::HandleCompletedMouseMovement | Game mode invalid when checking if mouse has reached its goal!"));
			return;
		}

		const ETeam MouseTeam = Mouse->GetTeam();

		MMGameMode->AddScore(MouseTeam);
		CompletedMice.Add(Mouse);

		if (MMGameMode->HasTeamWon(MouseTeam))
		{
			// Mouse was winning mouse, stop processing mice
			return;
		}
	}

	const bool bAnyRemainingMiceToProcess = MiceToProcessMovement.IsValidIndex(0);

	// There are still mice to process
	if (bAnyRemainingMiceToProcess)
	{
		// Process Next mouse
		ProcessMouse(MiceToProcessMovement[0]);
	}
	else
	{
		HandleMiceComplete();
	}
}

void AMM_GridManager::HandleMiceComplete()
{
	// Check if mice moved in the last process 
	if (CurrentProcessedMovedMiceCount > 0)
	{
		// Since mice moved, run through mice to process again to make sure any mice that can move, will move
		BeginProcessMice();
		return;
	}

	// Reached end, all mice processed
	if (MMGameMode)
	{
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::HandleMiceComplete | Turn ended for current player %i as %s, all mice processed"),
			MMGameMode->GetCurrentPlayer()->GetCurrentTeam(), *MMGameMode->GetCurrentPlayer()->GetName());

		if (CheckNoValidMoves())
		{
			// No more moves can be made, end game and work out winner or tie game
			MMGameMode->ForceEndNoMoves();
		}
		else
		{
			// Next player's turn
			MMGameMode->ProcessTurnComplete(CurrentPlayerProcessing);
		}
	}
	else
	{
		UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GridManager::HandleMiceComplete | Failed to get Gamemode when completing mice process!"));
	}
}

void AMM_GridManager::CleanupProcessedMouse(AMM_Mouse* Mouse)
{
	if (Mouse)
	{
		MiceToProcessMovement.Remove(Mouse);
		Mouse->MovementEndDelegate.RemoveDynamic(this, &AMM_GridManager::HandleCompletedMouseMovement);
	}
}

bool AMM_GridManager::CheckMouse(AMM_Mouse* Mouse)
{
	// Check if mouse valid
	if (Mouse)
	{
		return true;
	}

	UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GridManager::CheckMouse | Mouse not valid for processing!"));

	// Remove null ptr from array
	if (MiceToProcessMovement.IsValidIndex(0) && MiceToProcessMovement[0] == Mouse)
	{
		MiceToProcessMovement.RemoveAt(0);
	}

	// Mouse not valid, go on to next mouse
	HandleCompletedMouseMovement(Mouse);

	return false;
}

void AMM_GridManager::RemoveMouse(AMM_Mouse* Mouse)
{
	// Store variables
	const ETeam iTeam = Mouse->GetTeam();
	const FIntVector2D OriginalCoordinates = Mouse->GetCoordinates();

	// Remove mouse from previous location
	GridObject->SetGridElement(OriginalCoordinates, nullptr);
	RemoveMouseFromColumn(OriginalCoordinates.X, Mouse);

	// Cleanup from grid
	MiceTeams[iTeam].Remove(Mouse);
	Mice.Remove(Mouse);
}

void AMM_GridManager::SetMousePosition(AMM_Mouse* Mouse, const FIntVector2D& NewCoord)
{
	const FIntVector2D OriginalCoordinates = Mouse->GetCoordinates();

	const bool bSuccessfullyMoved = MoveGridElement(Mouse, NewCoord);
	if (!bSuccessfullyMoved)
	{
		return;
	}

	// Move mouse to new column
	RemoveMouseFromColumn(OriginalCoordinates.X, Mouse);
	AddMouseToColumn(NewCoord.X, Mouse);
}

bool AMM_GridManager::MoveGridElement(AMM_GridElement* GridElement, const FIntVector2D& NewCoord) const
{
	// Remove element from previous location and add to new location
	const bool bSuccessfullyMoved = GridObject->MoveGridElement(NewCoord, GridElement);

	return bSuccessfullyMoved;
}

void AMM_GridManager::RemoveMouseFromColumn(int Column, AMM_Mouse* Mouse)
{
	MiceColumns[Column].Remove(Mouse);

	// Check the column for this mouse's team
	const ETeam TeamToCheck = Mouse->GetTeam();
	// Remove from available list initially
	OccupiedTeamsPerColumn[Column].Remove(TeamToCheck);

	// Check through all mice in this column
	for (const AMM_Mouse* ColumnMouse : MiceColumns[Column])
	{
		// If a mouse in this column is the team to check
		if (ColumnMouse->GetTeam() == TeamToCheck)
		{
			// Add team to available column list for this column
			OccupiedTeamsPerColumn[Column].Add(TeamToCheck);
			break;
		}
	}
}

void AMM_GridManager::AddMouseToColumn(int Column, AMM_Mouse* Mouse)
{
	// Add to mice columns
	MiceColumns[Column].Add(Mouse);

	// Add team if not already in array for this column
	const ETeam TeamToAdd = Mouse->GetTeam();
	OccupiedTeamsPerColumn[Column].AddUnique(TeamToAdd);
}

bool AMM_GridManager::AdjustColumnInGridObject(int Column, EDirection Direction, AMM_GridElement*& LastElement) const
{
	// If direction is not up or down, no change will occur
	if (Direction != EDirection::E_UP && Direction != EDirection::E_DOWN)
	{
		return false;
	}

	// Check grid object valid
	if (!IsValid(GridObject))
	{
		UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GridManager::AdjustColumn | Grid Object not valid!"));
		return false;
	}

	LastElement = GridObject->MoveColumnElements(Column, Direction);
	return true;
}

void AMM_GridManager::AdjustColumn(int Column, EDirection Direction)
{
	AMM_GridElement* LastElement;
	const bool bSuccessfulColumnMovement = AdjustColumnInGridObject(Column, Direction, LastElement);
	if (!bSuccessfulColumnMovement)
	{
		return;
	}

	LastMovedColumn = Column;

	// Move Last element to correct position in world position
	if (LastElement)
	{
		const FIntVector2D CurrentSlot = LastElement->GetCoordinates();
		FVector NewLocation = CoordToWorldTransform(CurrentSlot).GetLocation();
		// Check column control exists
		if (ColumnControls.Contains(Column))
		{
			// Displace based on column's current offset to set the correct relative location
			NewLocation.Z += ColumnControls[Column]->GetActorLocation().Z - GetActorLocation().Z;
		}

		LastElement->SetActorLocation(NewLocation);

		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::AdjustColumn | Moving last element %s to %s"), *LastElement->GetName(), *CurrentSlot.ToString());
	}
}

bool AMM_GridManager::FindFreeSlotInDirection(FIntVector2D& CurrentPosition, const FIntVector2D& Direction) const
{
	return GridObject->FindFreeSlotInDirection(CurrentPosition, Direction);
}

bool AMM_GridManager::FindFreeSlotBelow(FIntVector2D& CurrentPosition) const
{
	return GridObject->FindFreeSlotBelow(CurrentPosition);
}

bool AMM_GridManager::FindFreeSlotAhead(FIntVector2D& CurrentPosition, EDirection Direction) const
{
	return GridObject->FindFreeSlotAhead(CurrentPosition, Direction);
}

bool AMM_GridManager::IsTeamInColumn(int Column, ETeam Team) const
{
	if (!OccupiedTeamsPerColumn.Contains(Column))
	{
		return false;
	}

	return OccupiedTeamsPerColumn[Column].Contains(Team);
}

TArray<int> AMM_GridManager::GetTeamColumns(ETeam Team) const
{
	TArray<int> AvailableColumns;

	int FailsafeColumn = -1;

	// For each column
	for (int x = 0; x < GridSize.X; x++)
	{
		// If the team is available in this column
		if (OccupiedTeamsPerColumn[x].Contains(Team))
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

	// If no columns were added and failsafe column was set
	if (AvailableColumns.Num() <= 0 && FailsafeColumn >= 0)
	{
		// Use failsafe column
		AvailableColumns.Add(FailsafeColumn);
	}

	return AvailableColumns;
}

bool AMM_GridManager::IsStalemate() const
{
	// Check each team for their mice count
	for (const auto& TeamMice : MiceTeams)
	{
		// If any team doesn't have one mice, its not a "stalemate"
		if (TeamMice.Value.Num() != 1)
		{
			return false;
		}
	}

	return true;
}

ETeam AMM_GridManager::GetWinningStalemateTeam(int& DistanceWonBy) const
{
	TMap<ETeam, int> TeamDistances;
	TArray<ETeam> MiceTeamsArray;
	MiceTeams.GenerateKeyArray(MiceTeamsArray);

	// Initalise distances for each team
	for (ETeam iTeam = ETeam::E_TEAM_A; iTeam < ETeam::E_MAX; ++iTeam)
	{
		TeamDistances.Add(iTeam, 0);
	}

	// Store distance for each team
	for (const auto& TeamMice : MiceTeams)
	{
		const ETeam CurrentTeam = TeamMice.Key;
		const TArray<AMM_Mouse*> CurrentMice = TeamMice.Value;
		if (CurrentMice.IsValidIndex(0))
		{
			const AMM_Mouse* RemainingMouse = CurrentMice[0];

			const int GridMultiply = CurrentTeam == ETeam::E_TEAM_A ? 0 : 1;
			// Get the distance from the team starting point
			const int Distance = FMath::Abs(((GridSize.X - 1) * GridMultiply) - RemainingMouse->GetCoordinates().X);

			TeamDistances[CurrentTeam] = Distance;
		}
	}

	// Check both team distances were stored
	if (TeamDistances.Num() >= 2)
	{
		// Check if same distance, tie situation
		if (TeamDistances[ETeam::E_TEAM_A] == TeamDistances[ETeam::E_TEAM_B])
		{
			DistanceWonBy = 0;
			// E_NONE meaning tie, ie no winner
			return ETeam::E_NONE;
		}

		// Check winning team based on further distance
		ETeam WinningTeam = ETeam::E_NONE;
		if (TeamDistances[ETeam::E_TEAM_A] > TeamDistances[ETeam::E_TEAM_B])
		{
			DistanceWonBy = TeamDistances[ETeam::E_TEAM_A] - TeamDistances[ETeam::E_TEAM_B];
			WinningTeam = ETeam::E_TEAM_A;
		}
		else
		{
			DistanceWonBy = TeamDistances[ETeam::E_TEAM_B] - TeamDistances[ETeam::E_TEAM_A];
			WinningTeam = ETeam::E_TEAM_B;
		}
		return WinningTeam;
	}

	UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GridManager::GetWinningStalemateTeam | Failed to get winning stalemate team, returned E_MAX!"));

	return ETeam::E_MAX;
}

bool AMM_GridManager::CheckNoValidMoves()
{
	ETeam CurrentTeam = ETeam::E_NONE;
	// NOTE: Assumes columns were added in correct order from left to right
	for (const TPair<int, AMM_ColumnControl*>& Column : ColumnControls)
	{
		// Check all elements in column
		bool bCurrentColumnFull = true;
		const int x = Column.Key;
		for (int y = 0; y < GridSize.Y; y++)
		{
			const AMM_GridElement* CurrentGridElement = GridObject->GetGridElement({x, y});
			if (!CurrentGridElement)
			{
				// Found empty slot, reset
				bCurrentColumnFull = false;
				CurrentTeam = ETeam::E_NONE;
				break;
			}
		}
		// Column was not full, go to next
		if (!bCurrentColumnFull)
		{
			continue;
		}

		// Check for next team
		ETeam NextTeam = CurrentTeam;
		++NextTeam;
		bool bSuccessColumn = true;
		for (const AMM_Mouse* CurrentMouse : MiceColumns[x])
		{
			if (!CurrentMouse)
			{
				continue;
			}
			// Check all mice are the team being looked for
			if (CurrentMouse->GetTeam() != NextTeam)
			{
				// One mouse was not the next team
				bSuccessColumn = false;
				break;
			}
		}

		// All mice were the next team, advance
		if (bSuccessColumn)
		{
			// Store next team as current as it was successful
			CurrentTeam = NextTeam;

			// Found full column of Team B after full team A
			if (CurrentTeam == ETeam::E_TEAM_B)
			{
				return true;
			}
		}
	}
	return false;
}

// ################################ Grid Debugging ################################

void AMM_GridManager::SetDebugVisualGrid(bool bEnabled)
{
	bDisplayDebugGrid = bEnabled;
}

void AMM_GridManager::ToggleDebugVisualGrid()
{
	SetDebugVisualGrid(!bDisplayDebugGrid);
}

bool AMM_GridManager::DebugCheckAllMiceProcessed() const
{
	for (ETeam CurrentTeam = ETeam::E_TEAM_A; CurrentTeam < ETeam::E_MAX; ++CurrentTeam)
	{
		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridManager::DebugCheckAllMiceProcessed | Checking ordered mice team %i"), CurrentTeam);
		for (const AMM_Mouse* Mouse : MiceTeams[CurrentTeam])
		{
			if (Mouse && !MiceToProcessMovement.Contains(Mouse))
			{
				UE_LOG(MiceMenEventLog, Error, TEXT("AMM_GridManager::DebugCheckAllMiceProcessed | MISSING Mouse at position %s"), *Mouse->GetCoordinates().ToString());
				return false;
			}
		}
	}
	return true;
}

void AMM_GridManager::DisplayDebugGrid() const
{
	// DEBUG FOR GRID VALUES
	// For each column
	for (int x = 0; x < GridSize.X; x++)
	{
		// For each row, add to column array
		for (int y = 0; y < GridSize.Y; y++)
		{
			const AMM_GridElement* gridElement = GridObject->GetGridElement({x, y});
			FLinearColor Color = FLinearColor::White;
			if (gridElement)
			{
				// Display yellow for block
				Color = FLinearColor::Yellow;

				// Display green for mice
				if (gridElement->IsA(AMM_Mouse::StaticClass()))
				{
					Color = FLinearColor::Green;
				}
			}
			UKismetSystemLibrary::DrawDebugSphere(
				GetWorld(),
				CoordToWorldTransform({x, y}).GetLocation() + FVector(-100, 0, 50),
				25.0f,
				6,
				Color,
				0.5,
				3);
		}
	}
}
