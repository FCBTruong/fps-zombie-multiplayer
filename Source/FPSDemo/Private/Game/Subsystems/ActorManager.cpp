// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Subsystems/ActorManager.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/TriggerBox.h"
#include "Engine/TargetPoint.h"
#include "NavigationSystem.h"
#include "Game/Items/Pickup/PickupItem.h"

// Sets default values
AActorManager::AActorManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AActorManager::BeginPlay()
{
	Super::BeginPlay();
    CurrentPickupId = 1000;
	UE_LOG(LogTemp, Warning, TEXT("ActorManager: BeginPlay called"));
	
    if (!TriggerBoxAreaA || !TriggerBoxAreaB)
    {
        UE_LOG(LogTemp, Warning, TEXT("ActorManager: Bomb areas not set up properly"));
	}
    else {
		UE_LOG(LogTemp, Warning, TEXT("ActorManager: Bomb areas set up properly"));
	}

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(),
        ATargetPoint::StaticClass(),
        FoundActors
    );
	
    for (AActor* Actor : FoundActors)
    {
        ATargetPoint* TargetPoint = Cast<ATargetPoint>(Actor);
        if (!TargetPoint)
        {
            continue;
        }

        if (TargetPoint->ActorHasTag(TEXT("DefenderWeaponInitPos"))) {
            DefenderWeaponInitPos = TargetPoint->GetActorLocation();
        }
        else if (TargetPoint->ActorHasTag(TEXT("AttackerWeaponInitPos"))) {
            AttackerWeaponInitPos = TargetPoint->GetActorLocation();
        }
    }
}

FVector AActorManager::GetSpikeStartLocation() const
{
    if (TargetPointSpike)
    {
        return TargetPointSpike->GetActorLocation();
    }
    else
    {
        return FVector::ZeroVector;
    }
}

APlayerStart* AActorManager::GetRandomStart(const TArray<APlayerStart*>& Starts)
{
    if (Starts.Num() == 0)
        return nullptr;

    // Collect unused
    TArray<APlayerStart*> Candidates;
    for (APlayerStart* Start : Starts)
    {
        if (!StartUsage.Contains(Start))
        {
            Candidates.Add(Start);
        }
	}

    if (Candidates.Num() == 0)
    {
        Candidates = Starts;
    }

    // Pick random
    APlayerStart* Chosen = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
	StartUsage.Add(Chosen);
    return Chosen;
}


void AActorManager::ResetPlayerStartsUsage()
{
    StartUsage.Empty();
}

APlayerStart* AActorManager::GetRandomAttackerStart()
{
    return GetRandomStart(AttackerStarts);
}

APlayerStart* AActorManager::GetRandomDefenderStart()
{
    return GetRandomStart(DefenderStarts);
}

AActorManager* AActorManager::Get(UObject* WorldContextObject)
{
    if (!WorldContextObject)
        return nullptr;

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
        return nullptr;

    for (TActorIterator<AActorManager> It(World); It; ++It)
    {
        return *It; // return first found
    }
    return nullptr;
}

FVector AActorManager::GetRandomHoldLocationNearBombSite(FName BombSiteName) const {
    TArray<ATargetPoint*> HoldPoints = (BombSiteName == FName(TEXT("A"))) ? HoldPointsA : HoldPointsB;

    if (HoldPoints.Num() > 0) {
        int32 RandomIndex = FMath::RandRange(0, HoldPoints.Num() - 1);
        return HoldPoints[RandomIndex]->GetActorLocation();
    }
    return FVector::ZeroVector;
}

FVector AActorManager::GetRandomScoutLocation() const
{
	return RandomLocationOnMap();
}

FVector AActorManager::RandomLocationOnMap() const {
    constexpr float Radius = 50000.f;
    constexpr int32 MaxTries = 20;

    UWorld* World = GetWorld();
    UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(World);
    if (!Nav) return FVector::ZeroVector;

    FVector Origin = FVector::ZeroVector;
    FNavLocation P;
    for (int32 i = 0; i < MaxTries; ++i)
    {
        if (Nav->GetRandomReachablePointInRadius(Origin, Radius, P))
        {
            return P.Location;
        }
    }
    return Origin;
}

void AActorManager::FindAndDestroyItemNode(int32 ItemOnMapId) {
    if (PickupItemsOnMap.Contains(ItemOnMapId)) {
        APickupItem* Pickup = PickupItemsOnMap[ItemOnMapId];
        if (Pickup) {
            Pickup->Destroy();
        }
        PickupItemsOnMap.Remove(ItemOnMapId);
    }
}

int32 AActorManager::GetNextItemOnMapId() {
    return CurrentPickupId++;
}

APickupItem* AActorManager::CreatePickupActor(FPickupData Data)
{
    APickupItem* Pickup = GetWorld()->SpawnActor<APickupItem>(
        APickupItem::StaticClass(),
        FVector::ZeroVector,
        FRotator::ZeroRotator
    );

    if (Pickup) {
        Pickup->SetData(Data);
        Pickup->GetItemMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        Pickup->GetItemMesh()->SetEnableGravity(true);
        Pickup->GetItemMesh()->SetLinearDamping(0.8f);
        Pickup->GetItemMesh()->SetAngularDamping(0.8f);
        Pickup->GetItemMesh()->SetPhysicsMaxAngularVelocityInDegrees(720.f);

        Pickup->SetActorLocation(Data.Location);
        PickupItemsOnMap.Add(Data.Id, Pickup);
    }
    return Pickup;
}

void AActorManager::CleanPickupItemsOnMap()
{
    for (auto& Elem : PickupItemsOnMap) {
        APickupItem* Pickup = Elem.Value;
        if (IsValid(Pickup))
        {
            Pickup->Destroy();
        }
    }
    PickupItemsOnMap.Empty();
}

APickupItem* AActorManager::GetPickupNode(int PickupId) {
    if (PickupItemsOnMap.Contains(PickupId)) {
        return PickupItemsOnMap[PickupId];
    }
    return nullptr;
}

APickupItem* AActorManager::GetPickupSpike() const {
    if (PickupSpike.IsValid()) {
        return PickupSpike.Get();
    }
    return nullptr;
}

void AActorManager::SetPickupSpike(APickupItem* SpikeItem) {
    PickupSpike = SpikeItem;
}

void AActorManager::RegisterPlayer(ABaseCharacter* Pawn) {
    if (!Pawn) return;
    RegisteredPlayers.Add(Pawn);
}

void AActorManager::UnregisterPlayer(ABaseCharacter* Pawn) {
    if (!Pawn) return;
    RegisteredPlayers.Remove(Pawn);
}
