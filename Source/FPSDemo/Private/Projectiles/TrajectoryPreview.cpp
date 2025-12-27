#include "Projectiles/TrajectoryPreview.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Game/GameManager.h"

ATrajectoryPreview::ATrajectoryPreview()
{
	PrimaryActorTick.bCanEverTick = true;

	SplineRef = CreateDefaultSubobject<USplineComponent>(TEXT("SplineThrow"));
	RootComponent = SplineRef;
}

void ATrajectoryPreview::BeginPlay()
{
	Super::BeginPlay();
}

void ATrajectoryPreview::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	BuildSplineMeshes();
}

USplineMeshComponent* ATrajectoryPreview::GetAMesh()
{
	return nullptr;
}


void ATrajectoryPreview::BuildSplineMeshes()
{
	if (!SplineRef) {
		UE_LOG(LogTemp, Error, TEXT("SplineRef is null in BuildSplineMeshes"));
		return;
	}

	int32 PointCount = SplineRef->GetNumberOfSplinePoints();
	UE_LOG(LogTemp, Verbose, TEXT("Building Spline Meshes, PointCount: %d"), PointCount);
	if (PointCount < 2) return;

	// Hide old meshes first
	for (USplineMeshComponent* Mesh : MeshPool)
	{
		if (Mesh)
			Mesh->SetVisibility(false);
	}

	for (int32 i = 0; i < PointCount - 1; i++)
	{
		USplineMeshComponent* Mesh = GetAMesh();
		if (!Mesh) continue;

		const FVector StartPos = SplineRef->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
		const FVector StartTangent = SplineRef->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local);
		const FVector EndPos = SplineRef->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::Local);
		const FVector EndTangent = SplineRef->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::Local);

		Mesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent, true);
	}
}