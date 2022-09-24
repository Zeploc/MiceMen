// Copyright Alex Coultas, Mice Men Example Project


#include "Grid/MM_GridElement.h"

#include "Grid/MM_GridManager.h"
#include "Gameplay/MM_ColumnControl.h"
#include "Base/MM_GameMode.h"
#include "MiceMen.h"

AMM_GridElement::AMM_GridElement()
{

}

void AMM_GridElement::SetupGridVariables(AMM_GridManager* InGridManager, AMM_GameMode* InMMGameMode, const FIntVector2D& InGridCoordinates)
{
	GridManager = InGridManager;
	Coordinates = InGridCoordinates;
	MMGameMode = InMMGameMode;
}

void AMM_GridElement::UpdateGridPosition(const FIntVector2D& NewGridCoordiantes)
{
	UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridElement::UpdateGridPosition | Updating grid element %s to %s"), *GetName(), *NewGridCoordiantes.ToString());

	// Set new position
	Coordinates = NewGridCoordiantes;

	// Checks grid manager is valid, should be set in SetupGridVariables()
	if (!GetGridManager())
	{
		return;
	}

	// Find new column from new location
	AMM_ColumnControl* NewColumn = nullptr;
	if (GridManager->GetColumnControls().Contains(Coordinates.X))
	{
		NewColumn = GridManager->GetColumnControls()[Coordinates.X];
	}

	// Check new column is different from current
	if (NewColumn != CurrentColumn)
	{
		// Remove from previous column
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		// Attach and store new column
		if (NewColumn)
		{
			AttachToActor(NewColumn, FAttachmentTransformRules::KeepWorldTransform);
		}
		CurrentColumn = NewColumn;

		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridElement::UpdateGridPosition | Switching to collumn %i"), Coordinates.X);
	}
}

void AMM_GridElement::CleanUp()
{
	GridManager = nullptr;
	MMGameMode = nullptr;
}

AMM_GridManager* AMM_GridElement::GetGridManager()
{
	// Should be set in SetupGridVariables()
	if (GridManager)
	{
		return GridManager;
	}

	// Defaults to grabbing from the game mode
	if (MMGameMode)
	{
		GridManager = MMGameMode->GetGridManager();
	}

	return GridManager;
}
