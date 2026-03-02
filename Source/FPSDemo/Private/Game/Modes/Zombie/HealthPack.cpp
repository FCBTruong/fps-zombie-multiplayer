// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Modes/Zombie/HealthPack.h"
#include "Game/GameManager.h"
#include "Shared/Data/GlobalDataAsset.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/Framework/ShooterGameState.h"

// Sets default values
AHealthPack::AHealthPack()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HealthPackMesh"));
	RootComponent = Mesh;

	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_PhysicsBody);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Mesh->SetGenerateOverlapEvents(true);
	Mesh->OnComponentBeginOverlap.AddDynamic(this, &AHealthPack::OnOverlapBegin);
}

// Called when the game starts or when spawned
void AHealthPack::BeginPlay()
{
	Super::BeginPlay();
	UGameManager* GM = UGameManager::Get(GetWorld());
	UGlobalDataAsset* GlobalData = GM->GlobalData;

	Mesh->SetStaticMesh(GlobalData->HealPackMesh);
	Mesh->SetRelativeScale3D(FVector(0.4, 0.4, 0.4));
}

// Called every frame
void AHealthPack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if !UE_SERVER
	// Rotate around Z axis (Yaw)
	AddActorLocalRotation(FRotator(0.f, RotateSpeedDegPerSec * DeltaTime, 0.f));
#endif
}

void AHealthPack::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsActorBeingDestroyed()) {
		return;
	}
	if (!HasAuthority())
	{
		return;
	}
	// Check if the overlapping actor is a player character
	ABaseCharacter* Character = Cast<ABaseCharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	float HealthAmount = 200.f; // can be configured in data asset if needed
	if (Character->Heal(HealthAmount))
	{
		AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>();
		if (!GS) return;
		GS->OnClaimedHealthPack(this);
		Destroy();
	}
}