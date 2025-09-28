#include "ShooterGameMode.h"
#include "ShooterGameState.h"
#include "Engine/World.h"
#include "WeaponBase.h"                 // include your weapon base class

void AShooterGameMode::StartPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("Game Started!"));
    Super::StartPlay();

    AShooterGameState* GS = GetGameState<AShooterGameState>();
    if (!GS) return;

   
    for (int32 i = 0; i < WeaponClasses.Num(); i++)
    {
        FVector SpawnLocation(200 * i, 0, 100);
        FRotator SpawnRotation = FRotator::ZeroRotator;

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AWeaponBase* NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClasses[i], SpawnLocation, SpawnRotation, Params);
        if (NewWeapon)
        {
            GS->WeaponsOnMap.Add(NewWeapon);
        }
    }
}
