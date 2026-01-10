// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/TeamEliminationMode.h"
#include <Kismet/GameplayStatics.h>
#include <GameFramework/PlayerStart.h>
#include "Game/TeamEliminationState.h"

ATeamEliminationMode::ATeamEliminationMode()
{

}


void ATeamEliminationMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

AActor* ATeamEliminationMode::ChoosePlayerStart_Implementation(AController* Player)
{
    return Super::ChoosePlayerStart_Implementation(Player); // fallback
}

void ATeamEliminationMode::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("GameMode StartRound in 2 seconds"));

    FTimerHandle StartRoundTimer;
    GetWorldTimerManager().SetTimer(
        StartRoundTimer,
        this,
        &ATeamEliminationMode::StartRound,
        1.0f,
        false
    );
}

void ATeamEliminationMode::StartRound()
{
    UE_LOG(LogTemp, Warning, TEXT("Start round"));

    if (bRoundInProgress) {
        return;
    }
	bRoundInProgress = true;
    ResetPlayers();
}


void ATeamEliminationMode::StartNextRound()
{
	UE_LOG(LogTemp, Warning, TEXT("Starting next round..."));
    GetWorld()->GetTimerManager().SetTimer(
        RoundStartTimer,
        this,
        &ATeamEliminationMode::StartRound,
        3.0f,          // Delay time
        false          // Do NOT loop
    );
}

void ATeamEliminationMode::EndGame(ETeamId WinningTeam)
{

}

void ATeamEliminationMode::OnCharacterKilled(class AController* Killer, ABaseCharacter* Victim, const UItemConfig* DamageCauser, bool bWasHeadShot)
{
    Super::OnCharacterKilled(Killer, Victim, DamageCauser);
}

void ATeamEliminationMode::CheckRoundEnd()
{
    
}
void ATeamEliminationMode::EndRound(ETeamId WinningTeam)
{
    
}
