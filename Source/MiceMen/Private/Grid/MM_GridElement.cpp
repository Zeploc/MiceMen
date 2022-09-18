// Copyright Alex Coultas, Mice Men Example Project


#include "Grid/MM_GridElement.h"

#include "Grid/MM_GridManager.h"
#include "Gameplay/MM_ColumnControl.h"
#include "Base/MM_GameMode.h"
#include "MiceMen.h"

AMM_GridElement::AMM_GridElement()
{

}

void AMM_GridElement::SetupGridInfo(AMM_GridManager* _GridManager, AMM_GameMode* _MMGameMode, FIntVector2D _GridCoordinates)
{
	GridManager = _GridManager;
	Coordinates = _GridCoordinates;
	MMGameMode = _MMGameMode;
}

void AMM_GridElement::UpdateGridPosition(FIntVector2D _NewGridCoordiantes)
{
	UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridElement::UpdateGridPosition | Updating grid element %s to %s"), *GetName(), *_NewGridCoordiantes.ToString());

	Coordinates = _NewGridCoordiantes;

	// Checks grid manager is valid, should be set in setup grid info
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

	// Switch to new column
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

}

AMM_GridManager* AMM_GridElement::GetGridManager()
{
	// Should be set in SetupGridInfo()
	if (GridManager)
	{
		return GridManager;
	}

	if (!GetWorld())
	{
		return nullptr;
	}

	// Defaults to grabbing from the gamemode
	if (AMM_GameMode* MMGamemode = GetWorld()->GetAuthGameMode<AMM_GameMode>())
	{
		GridManager = MMGamemode->GetGridManager();
	}

	return GridManager;
}
