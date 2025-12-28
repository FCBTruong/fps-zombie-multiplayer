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
    AMyPlayerState* PS = Player->GetPlayerState<AMyPlayerState>();

	if (!PS) {
        UE_LOG(LogTemp, Warning, TEXT("PlayerState is null in ChoosePlayerStart_Implementation"));
        return Super::ChoosePlayerStart_Implementation(Player); // fallback
	}


    FName TeamId = PS->GetTeamID();

    TArray<AActor*> FoundStarts;
    UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), FoundStarts);

    for (AActor* Actor : FoundStarts)
    {
        APlayerStart* Start = Cast<APlayerStart>(Actor);
        if (!Start)
        {
            continue;
        }
		UE_LOG(LogTemp, Warning, TEXT("Found PlayerStart with tag: %s"), *Start->PlayerStartTag.ToString());
		UE_LOG(LogTemp, Warning, TEXT("Players TeamId: %s"), *TeamId.ToString());

        if (Start->PlayerStartTag == TeamId)
        {
            return Start;
        }
    }

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

    // test create bot AI 
	SpawnBot("B");
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

void ATeamEliminationMode::EndGame(FName WinningTeam)
{
    FString WinningTeamStr = WinningTeam.ToString();
    UE_LOG(LogTemp, Warning, TEXT("Game Over! Team %s wins!"), *WinningTeamStr);
    // Implement additional end game logic here (e.g., display UI, reset game, etc.)
}

void ATeamEliminationMode::NotifyPlayerKilled(class AController* Killer, class AController* Victim, UItemConfig* DamageCauser, bool bWasHeadShot)
{
    Super::NotifyPlayerKilled(Killer, Victim, DamageCauser);
}

void ATeamEliminationMode::CheckRoundEnd()
{
    
}
void ATeamEliminationMode::EndRound(FName WinningTeam)
{
    
}
