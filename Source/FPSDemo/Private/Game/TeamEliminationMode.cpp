// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/TeamEliminationMode.h"
#include <Kismet/GameplayStatics.h>
#include <GameFramework/PlayerStart.h>
#include "Game/TeamEliminationState.h"

ATeamEliminationMode::ATeamEliminationMode()
{
    bRoundInProgress = true;
}

void ATeamEliminationMode::AddPlayer(APlayerController* NewPlayer)
{
    if (!NewPlayer) return;

    AMyPlayerState* PS = NewPlayer->GetPlayerState<AMyPlayerState>();
    if (!PS) return;

    FName AssignedTeam;

    if (TeamA.Num() > TeamB.Num())
    {
        AssignedTeam = "B";
        TeamB.Add(NewPlayer);
    }
    else
    {
        AssignedTeam = "A";
        TeamA.Add(NewPlayer);
    }

    PS->SetTeamID(AssignedTeam);
}

void ATeamEliminationMode::PostLogin(APlayerController* NewPlayer)
{
	// decide team for the new player

    AddPlayer(NewPlayer);
	Super::PostLogin(NewPlayer);

	// log number of players in each team
	UE_LOG(LogTemp, Warning, TEXT("Team A has %d players."), TeamA.Num());
	UE_LOG(LogTemp, Warning, TEXT("Team B has %d players."), TeamB.Num());
}

AActor* ATeamEliminationMode::ChoosePlayerStart_Implementation(AController* Player)
{
    AMyPlayerState* PS = Player->GetPlayerState<AMyPlayerState>();


    FName TeamId = PS->GetTeamID();

    TArray<AActor*> FoundStarts;
    UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), FoundStarts);

    for (AActor* Actor : FoundStarts)
    {
        APlayerStart* Start = Cast<APlayerStart>(Actor);
        if (!Start)
            continue;

        if (Start->PlayerStartTag == TeamId)
        {
            return Start;
        }
    }

    return Super::ChoosePlayerStart_Implementation(Player); // fallback
}

void ATeamEliminationMode::StartRound()
{
	bRoundInProgress = true;
    ResetPlayers();
}

void ATeamEliminationMode::ResetPlayers()
{
    for (APlayerController* PC : TeamA)
    {
        if (PC)
        {
            if (APawn* OldPawn = PC->GetPawn())
            {
                OldPawn->Destroy();     // Required!
            }
            RestartPlayer(PC);          // Will now actually respawn at a PlayerStart
        }
    }

    for (APlayerController* PC : TeamB)
    {
        if (PC)
        {
            if (APawn* OldPawn = PC->GetPawn())
            {
                OldPawn->Destroy();
            }
            RestartPlayer(PC);
        }
    }
}

void ATeamEliminationMode::RestartPlayer(AController* NewPlayer)
{
    Super::RestartPlayer(NewPlayer);
    if (AMyPlayerState* PS = NewPlayer->GetPlayerState<AMyPlayerState>())
    {
        PS->SetIsAlive(true);
    }
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

void ATeamEliminationMode::NotifyPlayerKilled(class AController* Killer, class AController* Victim, class AActor* DamageCauser)
{
    Super::NotifyPlayerKilled(Killer, Victim, DamageCauser);
	UE_LOG(LogTemp, Warning, TEXT("NotifyPlayerKilled called in TeamEliminationMode"));
    AMyPlayerState* KillerPS = Killer ? Killer->GetPlayerState<AMyPlayerState>() : nullptr;
    AMyPlayerState* VictimPS = Victim ? Victim->GetPlayerState<AMyPlayerState>() : nullptr;
    if (!VictimPS) {
		UE_LOG(LogTemp, Warning, TEXT("VictimPS is null in NotifyPlayerKilled"));
        return;
    }
	VictimPS->SetIsAlive(false);
    
    if (bRoundInProgress) {
        CheckRoundEnd();
    }
}

void ATeamEliminationMode::CheckRoundEnd()
{
    bool bTeamAAllDead = true;
    bool bTeamBAllDead = true;

    for (APlayerController* PC : TeamA)
    {
        if (AMyPlayerState* PS = PC->GetPlayerState<AMyPlayerState>())
        {
            if (PS->IsAlive())
            {
                bTeamAAllDead = false;
                break;
            }
        }
    }

    for (APlayerController* PC : TeamB)
    {
        if (AMyPlayerState* PS = PC->GetPlayerState<AMyPlayerState>())
        {
            if (PS->IsAlive())
            {
                bTeamBAllDead = false;
                break;
            }
        }
    }

    // Determine winner
    if (bTeamAAllDead && !bTeamBAllDead)
    {
        EndRound("B");
    }
    else if (bTeamBAllDead && !bTeamAAllDead)
    {
        EndRound("A");
    }
}
void ATeamEliminationMode::EndRound(FName WinningTeam)
{
    ATeamEliminationState* GS = GetGameState<ATeamEliminationState>();
    if (!GS) {
        return;
	}
	bRoundInProgress = false;
	UE_LOG(LogTemp, Warning, TEXT("Round Over! Team %s wins the round!"), *WinningTeam.ToString());
    if (WinningTeam == "A")
    {
		GS->TeamAScore++;
    }
    else
    {
		GS->TeamBScore++;
    }

    // Match victory
    if (GS->TeamAScore >= MaxRoundsToWin)
    {
        EndGame("A");
        return;
    }
    if (GS->TeamBScore >= MaxRoundsToWin)
    {
        EndGame("B");
        return;
    }

    StartNextRound();
}

