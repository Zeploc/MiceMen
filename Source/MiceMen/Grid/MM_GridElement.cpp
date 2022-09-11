// Copyright Alex Coultas, Mice Men Example Project


#include "Grid/MM_GridElement.h"

#include "MM_GridManager.h"
#include "Gameplay/MM_ColumnControl.h"
#include "MiceMen.h"

AMM_GridElement::AMM_GridElement()
{

}
void AMM_GridElement::SetupGridInfo(AMM_GridManager* _GridManager, FIntVector2D _GridCoordinates)
{
	GridManager = _GridManager;
	Coordinates = _GridCoordinates;
}

void AMM_GridElement::UpdateGridPosition(FIntVector2D _NewGridCoordiantes)
{
	UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridElement::UpdateGridPosition | Updating grid element %s to %s"), *GetName(), *_NewGridCoordiantes.ToString());

	Coordinates = _NewGridCoordiantes;

	// TODO: Add catch to find grid manager
	// Should be set in setup grid info
	if (!GridManager)
		return;

	// Find new column from new location
	AMM_ColumnControl* NewColumn = nullptr;
	if (GridManager->GetColumnControls().Contains(Coordinates.X))
	{
		NewColumn = GridManager->GetColumnControls()[Coordinates.X];
	}

	// Switch to new column
	if (NewColumn != CurrentColumn)
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		if (NewColumn)
			AttachToActor(NewColumn, FAttachmentTransformRules::KeepWorldTransform);
		CurrentColumn = NewColumn;

		UE_LOG(MiceMenEventLog, Display, TEXT("AMM_GridElement::UpdateGridPosition | Switching to collumn %i"), Coordinates.X);
	}
}

void AMM_GridElement::CleanUp()
{

}
