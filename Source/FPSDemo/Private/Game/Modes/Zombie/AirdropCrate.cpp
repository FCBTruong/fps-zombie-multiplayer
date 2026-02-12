// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Modes/Zombie/AirdropCrate.h"
#include "Components/StaticMeshComponent.h"
#include "Game/GameManager.h"
#include "Shared/Data/GlobalDataAsset.h"
#include "Game/Modes/Zombie/ZombieMode.h"

// Sets default values
AAirdropCrate::AAirdropCrate()
{
	bReplicates = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AirdropCrateMesh"));
	RootComponent = Mesh;

	Mesh->SetSimulatePhysics(true);
	Mesh->SetEnableGravity(true);
	Mesh->SetLinearDamping(3.0f);   // Increase to fall slower (2–8 typical)
	Mesh->SetAngularDamping(2.0f);

	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_PhysicsBody);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Mesh->SetGenerateOverlapEvents(true);
	Mesh->OnComponentBeginOverlap.AddDynamic(this, &AAirdropCrate::OnOverlapBegin);
}

// Called when the game starts or when spawned
void AAirdropCrate::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("AirdropCrate::BeginPlay called"));

	// load mesh asset
	UGameManager* GM = UGameManager::Get(GetWorld());
	UGlobalDataAsset* GlobalData = GM->GlobalData;
	if (!GlobalData)
	{
		UE_LOG(LogTemp, Error, TEXT("AAirdropCrate::BeginPlay: GlobalData is null"));
		return;
	}

	Mesh->SetStaticMesh(GlobalData->CrateMesh);
	Mesh->SetRelativeScale3D(FVector(3, 3, 3));
}

void AAirdropCrate::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("AirdropCrate::OnOverlapBegin called"));
	if (IsActorBeingDestroyed()) {
		return;
	}
	if (!HasAuthority())
	{
		return;
	}
	if (bIsClaimed)
	{
		return;
	}
	// dispatch to game mode
	ABaseCharacter* Character = Cast<ABaseCharacter>(OtherActor);
	if (Character && Character->IsAlive() && Character->IsCharacterRole(ECharacterRole::Human))
	{
		bIsClaimed = true;
		OnAirdropClaimed.Broadcast(this, Character);
		UE_LOG(LogTemp, Warning, TEXT("AirdropCrate claimed by character"));
	}
}


