// Copyright Alex Coultas, Mice Men Example Project


#include "Grid/MM_GridElement.h"

AMM_GridElement::AMM_GridElement()
{

}
void AMM_GridElement::SetupGridInfo(FIntVector2D _GridCoordinates)
{
	Coordinates = _GridCoordinates;
}

void AMM_GridElement::UpdateGridPosition(FIntVector2D _NewGridCoordiantes)
{
	Coordinates = _NewGridCoordiantes;
}

void AMM_GridElement::CleanUp()
{

}
