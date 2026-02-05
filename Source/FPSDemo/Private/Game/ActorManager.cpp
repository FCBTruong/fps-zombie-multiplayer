// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ActorManager.h"
#include "EngineUtils.h"
#include <Kismet/GameplayStatics.h>
#include "GameFramework/PlayerStart.h"
#include "Engine/TriggerBox.h"
#include "Engine/TargetPoint.h"
#include "NavigationSystem.h"

// Sets default values
AActorManager::AActorManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AActorManager::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("ActorManager: BeginPlay called"));
	
    if (!TriggerBoxAreaA || !TriggerBoxAreaB)
    {
        UE_LOG(LogTemp, Warning, TEXT("ActorManager: Bomb areas not set up properly"));
	}
    else {
		UE_LOG(LogTemp, Warning, TEXT("ActorManager: Bomb areas set up properly"));
	}

    const ENetRole LocalRole = GetLocalRole();
    UE_LOG(LogTemp, Warning, TEXT("BeginPlay: %s | Role=%d (%s) | World=%s"),
        *GetName(),
        (int32)LocalRole,
        LocalRole == ROLE_Authority ? TEXT("Authority") : TEXT("NonAuthority"),
        *GetWorld()->GetName()
    );

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
        UE_LOG(LogTemp, Warning, TEXT("ActorManager: TargetPointSpike is not set"));
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

    UE_LOG(LogTemp, Warning, TEXT("No hold points available for bomb site: %s"), *BombSiteName.ToString());
    return FVector::ZeroVector;
}

FVector AActorManager::GetRandomScoutLocation() const
{
	return RandomLocationOnMap();
}

FVector AActorManager::RandomLocationOnMap() const {
	auto WorldContext = Cast<UObject>(this);
    constexpr float Radius = 50000.f;
    constexpr int32 MaxTries = 20;

    if (!WorldContext) return FVector::ZeroVector;

    UWorld* World = WorldContext->GetWorld();
    if (!World) return FVector::ZeroVector;

    UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(World);
    if (!Nav) return FVector::ZeroVector;

    FVector Origin = FVector::ZeroVector;
    if (const AActor* AsActor = Cast<AActor>(WorldContext))
    {
        Origin = AsActor->GetActorLocation();
    }

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
