#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrajectoryPreview.generated.h"

UCLASS()
class FPSDEMO_API ATrajectoryPreview : public AActor
{
	GENERATED_BODY()

public:
	ATrajectoryPreview();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USplineComponent* SplineRef;

	// pool of spline meshes
	UPROPERTY()
	TArray<class USplineMeshComponent*> MeshPool;

	UPROPERTY(EditDefaultsOnly, Category = "Trajectory")
	float MeshScale = 0.06f;

	// get or create a spline mesh from the pool
	class USplineMeshComponent* GetAMesh();
	void BuildSplineMeshes();
};
