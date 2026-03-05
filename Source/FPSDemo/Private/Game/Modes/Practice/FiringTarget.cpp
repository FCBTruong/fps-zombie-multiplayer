// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Modes/Practice/FiringTarget.h"

// Sets default values
AFiringTarget::AFiringTarget()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AFiringTarget::BeginPlay()
{
	Super::BeginPlay();

	InitialLocation = GetActorLocation();
	TArray<UStaticMeshComponent*> MeshComponents;
	GetComponents<UStaticMeshComponent>(MeshComponents);

	for (UStaticMeshComponent* MeshComp : MeshComponents)
	{
		if (MeshComp && MeshComp->GetName() == TEXT("Target"))
		{
			TargetMesh = MeshComp;
			break;
		}
	}
}

float AFiringTarget::TakeDamage(
	float DamageAmount,
	struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator,
	AActor* DamageCauser
)
{
	OnHit();
	UE_LOG(LogTemp, Warning, TEXT("Target hit!"));
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

// Called every frame
void AFiringTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;
	float Offset = FMath::Sin(RunningTime * MoveSpeed) * MoveAmplitude;
	FVector NewLocation = InitialLocation + FVector(Offset, 0.f, 0.f);
	SetActorLocation(NewLocation);
}

void AFiringTarget::UpdateRotation()
{
	RotationElapsed += 0.016f;
	float Alpha = FMath::Clamp(RotationElapsed / RotationDuration, 0.f, 1.f);
	FRotator NewRotation = FMath::Lerp(HitRotation, OriginalRotation, Alpha);
	TargetMesh->SetRelativeRotation(NewRotation);

	if (Alpha >= 1.f)
	{
		GetWorldTimerManager().ClearTimer(RotationTimer);
		bIsActive = true;
	}
}

void AFiringTarget::OnHit()
{
	if (!TargetMesh || !bIsActive)
	{
		return;
	}

	bIsActive = false;

	OriginalRotation = TargetMesh->GetRelativeRotation();
	HitRotation = OriginalRotation + FRotator(90.f, 0.f, 0.f);

	// Instantly rotate backward
	TargetMesh->SetRelativeRotation(HitRotation);
	RotationElapsed = 0.f;

	// Start rotating back
	GetWorldTimerManager().SetTimer(
		ReturnDelayTimer,
		this,
		&AFiringTarget::StartReturnRotation,
		1.0f,
		false
	);
}


void AFiringTarget::StartReturnRotation()
{
	GetWorldTimerManager().SetTimer(
		RotationTimer,
		this,
		&AFiringTarget::UpdateRotation,
		0.016f,
		true
	);
}
