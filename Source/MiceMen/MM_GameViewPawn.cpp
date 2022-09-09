// Copyright Alex Coultas, Mice Men Example Project


#include "MM_GameViewPawn.h"

#include "CineCameraComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "MM_PlayerController.h"
#include "Gameplay/MM_ColumnControl.h"
#include "MM_GameMode.h"
#include "Grid/MM_GridManager.h"

// Sets default values
AMM_GameViewPawn::AMM_GameViewPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GameCamera = CreateDefaultSubobject<UCineCameraComponent>(TEXT("Game Camera"));
}

// Called to bind functionality to input
void AMM_GameViewPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(FName("Grab"), IE_Pressed, this, &AMM_GameViewPawn::BeginGrab);
	PlayerInputComponent->BindAction(FName("Grab"), IE_Released, this, &AMM_GameViewPawn::EndGrab);

}

// Called when the game starts or when spawned
void AMM_GameViewPawn::BeginPlay()
{
	Super::BeginPlay();
	
	MMGameMode = GetWorld()->GetAuthGameMode<AMM_GameMode>();
}

void AMM_GameViewPawn::PossessedBy(AController* _NewController)
{
	Super::PossessedBy(_NewController);

	MMPlayerController = Cast<AMM_PlayerController>(_NewController);
}

// Called every frame
void AMM_GameViewPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleGrab();
}

void AMM_GameViewPawn::BeginGrab()
{
	if (!MMPlayerController)
		return;

	// Deproject mouse cursor to world
	FVector WorldLocation, WorldDirection;
	if (!MMPlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
		// Failed to deproject
		return;

	// Get hit at mouse position in world on interactable channel
	FHitResult InteractHit;
	if (!GetWorld()->LineTraceSingleByChannel(InteractHit, WorldLocation, WorldLocation + WorldDirection * InteractTraceDistance, ECC_GameTraceChannel1))
		// Failed hit
		return;

	CurrentColumn = Cast<AMM_ColumnControl>(InteractHit.GetActor());
	if (CurrentColumn)
	{
		CurrentColumn->BeginGrab();
		HitColumnOffset = CurrentColumn->GetActorLocation() - InteractHit.Location;
	}
}

void AMM_GameViewPawn::EndGrab()
{
	if (CurrentColumn)
		CurrentColumn->EndGrab();
	CurrentColumn = nullptr;
}

void AMM_GameViewPawn::HandleGrab()
{
	if (!CurrentColumn || !MMPlayerController)
		return;

	// Deproject mouse cursor to world
	FVector WorldLocation, WorldDirection;
	if (!MMPlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
		// Failed to deproject
		return;

	float T;
	FVector IntersectionLocation;
	if (!UKismetMathLibrary::LinePlaneIntersection_OriginNormal(WorldLocation, WorldLocation + WorldDirection * InteractTraceDistance, CurrentColumn->GetActorLocation(), CurrentColumn->GetActorForwardVector() * -1, T, IntersectionLocation))
		return;

	FVector NewLocation = CurrentColumn->GetActorLocation();
	NewLocation.Z = IntersectionLocation.Z + HitColumnOffset.Z;
	CurrentColumn->UpdatePreviewLocation(NewLocation);
}

AMM_GridManager* AMM_GameViewPawn::GetGridManager()
{
	if (GridManager)
		return GridManager;

	if (!MMGameMode)
		return nullptr;

	GridManager = MMGameMode->GetGridManager();


	return GridManager;
}
