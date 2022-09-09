// Copyright Alex Coultas, Mice Men Example Project


#include "Gameplay/MM_ColumnControl.h"
#include "Components/BoxComponent.h"
#include "Grid/MM_GridManager.h"

// Sets default values
AMM_ColumnControl::AMM_ColumnControl()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	SetRootComponent(SceneRoot);

	GrabbableBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Grabbable Box"));
	GrabbableBox->SetupAttachment(RootComponent);
	GrabbableBox->SetCollisionProfileName(FName("GridColumn"));
}

// Called when the game starts or when spawned
void AMM_ColumnControl::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMM_ColumnControl::SetupColumn(int _ColumnID, AMM_GridManager* _GridManager)
{
	ControllingColumn = _ColumnID;
	GridManager = _GridManager;

	float ColumnHeight = _GridManager->GridElementHeight * _GridManager->GridSize.Y;
	FVector BoxSize = FVector(50, _GridManager->GridElementWidth / 2.0f, ColumnHeight / 2.0);
	
	GrabbableBox->SetBoxExtent(BoxSize);
	GrabbableBox->SetRelativeLocation(FVector(0, 0, ColumnHeight / 2));
}

// Called every frame
void AMM_ColumnControl::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

