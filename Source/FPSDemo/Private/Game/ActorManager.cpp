// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ActorManager.h"
#include "EngineUtils.h"

// Sets default values
AActorManager::AActorManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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
}

// Called every frame
void AActorManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector AActorManager::GetSpikeStartLocation()
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

    // Build usage map if empty
    for (APlayerStart* PS : Starts)
    {
        if (PS && !StartUsage.Contains(PS))
        {
            StartUsage.Add(PS, false);
        }
    }

    // Collect unused
    TArray<APlayerStart*> Candidates;
    for (auto& Pair : StartUsage)
    {
        if (!Pair.Value && Starts.Contains(Pair.Key))
        {
            Candidates.Add(Pair.Key);
        }
    }

    // If none unused left reset
    if (Candidates.Num() == 0)
    {
        for (APlayerStart* PS : Starts)
            StartUsage[PS] = false;

        Candidates = Starts;
    }

    // Pick random
    APlayerStart* Chosen = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
    StartUsage[Chosen] = true;

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
