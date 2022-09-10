// Copyright Alex Coultas, Mice Men Example Project


#include "Grid/MM_GridManager.h"

#include "Kismet/GameplayStatics.h"

#include "MM_GridElement.h"
#include "MM_GridBlock.h"
#include "Gameplay/MM_Mouse.h"
#include "Gameplay/MM_ColumnControl.h"
#include "MM_GridObject.h"

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

	RebuildGrid();

}
// Called every frame
void AMM_GridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bDebugGridEnabled)
		DebugVisualiseGrid();
}


void AMM_GridManager::SetupGrid(FIntVector2D _GridSize)
{
	GridSize = _GridSize;
}

void AMM_GridManager::RebuildGrid()
{
	GridCleanUp();

	if (GridSize.X < 2 || GridSize.Y < 2)
	{
		UE_LOG(LogTemp, Error, TEXT("GRID TOO SMALL, CANNOT SETUP"));
		return;
	}

	GridObject = NewObject<UMM_GridObject>(UMM_GridObject::StaticClass());
	GridObject->SetupGrid(GridSize);

	// Remainder of divide by 2, either 0 or 1
	GapSize = GridSize.X % 2;
	// Team size is the amount without the gap, halved
	TeamSize = (GridSize.X - GapSize) / 2;

	PopulateGrid();
	PopulateMice();
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
		AMM_ColumnControl* NewColumnControl = GetWorld()->SpawnActor<AMM_ColumnControl>(ColumnControlClass, ColumnTransform);
		NewColumnControl->SetupColumn(x, this);
		ColumnControls.Add(x, NewColumnControl);
		

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
			isPresetBlockFilled = true;
	}

	// Initial testing random bool for placement
	if ((FMath::RandBool() && !IsPresetCenterBlock) || isPresetBlockFilled)
	{
		// Create new Grid block object and add to column array
		AMM_GridBlock* NewGridBlock = GetWorld()->SpawnActor<AMM_GridBlock>(GridBlockClass, GridElementTransform);
		if (!NewGridBlock)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create Grid Block!"));
			return;
		}
		NewGridBlock->SetupGridInfo(this, _NewCoord);
		if (NewColumnControl)
			NewGridBlock->AttachToActor(NewColumnControl, FAttachmentTransformRules::KeepWorldTransform);
		GridObject->SetGridElement(_NewCoord, NewGridBlock);
	}
	else
	{
		// Set element as empty
		GridObject->SetGridElement(_NewCoord, nullptr);
	}
}

void AMM_GridManager::PopulateMice()
{
	FIntVector2D TeamRanges[] = {
		FIntVector2D(0, TeamSize - 1),
		FIntVector2D(TeamSize + GapSize, GridSize.X - 1)
	};

	for (int iTeam = 0; iTeam < 2; iTeam++)
	{
		// Add initial team array
		TeamMice.Add(iTeam, TArray<AMM_Mouse*>());
		// Add mice for team
		for (int iMouse = 0; iMouse < InitialMiceCount; iMouse++)
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

			GridObject->SetGridElement(NewRandomMousePosition, NewMouse);


			// Update FreeSlots array
			GridObject->RegenerateFreeSlots();
		}
	}

}

TArray<FVector> AMM_GridManager::PathFromCoordToWorld(TArray<FIntVector2D> _CoordPath)
{
	TArray<FVector> NewWorldPath;
	for (FIntVector2D _Coord : _CoordPath)
	{		
		NewWorldPath.Add(GetWorldTransformFromCoord(_Coord).GetLocation());
	}
	return NewWorldPath;
}

FTransform AMM_GridManager::GetWorldTransformFromCoord(FIntVector2D _Coords)
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


void AMM_GridManager::ProcessMice()
{
	TArray<int> OrderedTeamsToProcess;
	// TODO Link to whos turn
	OrderedTeamsToProcess.Add(0);
	OrderedTeamsToProcess.Add(1);


	for (int iTeam : OrderedTeamsToProcess)
	{
		// Mice for this team not added
		if (!TeamMice.Contains(iTeam))
			continue;

		TArray<AMM_Mouse*> CompletedTeamMice;

		for (AMM_Mouse* Mouse : TeamMice[iTeam])
		{
			// Get Path
			TArray<FIntVector2D> ValidPath = GridObject->GetValidPath(Mouse->GetCoordinates(), Mouse->GetTeam() == 0 ? 1 : -1);

			// TODO: DEBUG
			auto colour = FLinearColor::MakeRandomColor();
			for (int i = 0; i < ValidPath.Num(); i++)
			{
				UKismetSystemLibrary::DrawDebugBox(GetWorld(), GetWorldTransformFromCoord(ValidPath[i]).GetLocation() + FVector(0, 0, 50), FVector(40 * ((float)i / (float)10) + 5), colour, FRotator::ZeroRotator, 5, 3);
			}

			// TODO: Change to event based, ie one mouse goes at a time, Should solve below problem
			// TODO: Order could have one mouse be blocked by another that can move
			// Need to take into account mice that haven't moved in path
			// Also if a second team movement then opens movement for the previously moved team?
			// Make movement
			Mouse->MoveAlongPath(PathFromCoordToWorld(ValidPath));
			GridObject->SetGridElement(Mouse->GetCoordinates(), nullptr);
			FIntVector2D FinalPosition = ValidPath.Last();

			// If the mouse is at the end
			if ((FinalPosition.X <= 0 && iTeam == 1) || (FinalPosition.X >= GridSize.X - 1 && iTeam == 0))
			{
				Mouse->MouseComplete();
				CompletedTeamMice.Add(Mouse);
			}
			// Not at the end, set coordinates to end path position
			else
				GridObject->SetGridElement(FinalPosition, Mouse);

			// Update FreeSlots array
			GridObject->RegenerateFreeSlots();
		}

		for (AMM_Mouse* CompleteMouse : CompletedTeamMice)
		{
			TeamMice[iTeam].Remove(CompleteMouse);
			Mice.Remove(CompleteMouse);
		}
	}
	
}


void AMM_GridManager::AdjustColumn(int _Column, int _Direction)
{
	FIntVector2D LastColumnCoord = { _Column, 0 };
	if (_Direction > 0)
		LastColumnCoord = { _Column, GridSize.Y - 1 };

	// Find Last element and remove from grid
	AMM_GridElement* LastElement = GridObject->GetGridElement(LastColumnCoord);
	GridObject->SetGridElement(LastColumnCoord, nullptr);

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
			}
		}

	}

	// Update FreeSlots array
	GridObject->RegenerateFreeSlots();

	ProcessMice();
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