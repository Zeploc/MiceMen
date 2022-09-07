// Copyright Alex Coultas, Mice Men Example Project


#include "Grid/MM_GridInfo.h"
#include "Gameplay/MM_Mouse.h"
#include "Grid/MM_GridBlock.h"

UMM_GridInfo::UMM_GridInfo()
{

}
void UMM_GridInfo::SetupGridInfo(FIntVector _GridCoordinates)
{
	Coordinates = _GridCoordinates;
}

void UMM_GridInfo::SetGridBlock(AMM_GridBlock* _Block)
{
	Block = _Block;
}

bool UMM_GridInfo::IsEmpty()
{
	if (IsValid(Mouse))
		return false;
	if (IsValid(Block))
		return false;

	return true;
}
