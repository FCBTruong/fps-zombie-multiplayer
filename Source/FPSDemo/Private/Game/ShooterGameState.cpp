// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ShooterGameState.h"
#include <Net/UnrealNetwork.h>
#include "Pickup/PickupItem.h"
#include "Controllers/MyPlayerController.h"

AShooterGameState::AShooterGameState()
{
    // Enable replication
    bReplicates = true;
}

void AShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
}

TArray<FPickupData> AShooterGameState::GetItemsOnMap() const
{
    TArray<FPickupData> OutItems;
    ItemsOnMap.GenerateValueArray(OutItems);
    return OutItems;
}


void AShooterGameState::MulticastKillNotify_Implementation(AMyPlayerState* Killer, AMyPlayerState* Victim, UWeaponData* DamageCauser)
{
    const FString KillerName = Killer ? Killer->GetPlayerName() : TEXT("Unknown");
    const FString VictimName = Victim ? Victim->GetPlayerName() : TEXT("Unknown");

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC);
        if (!MyPC) {
            return;
        }
        if (!MyPC->PlayerUI) {
            return;
		}

        UTexture2D* WeaponTex = nullptr;
        if (DamageCauser)
        {
			WeaponTex = DamageCauser->Icon;
        }

		MyPC->PlayerUI->NotifyKill(KillerName, VictimName, WeaponTex, false);
    }
}