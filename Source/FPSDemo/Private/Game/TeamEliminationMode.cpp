// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/TeamEliminationMode.h"
#include <Kismet/GameplayStatics.h>
#include <GameFramework/PlayerStart.h>
#include "Game/TeamEliminationState.h"
#include "Weapons/WeaponData.h"

ATeamEliminationMode::ATeamEliminationMode()
{
    bRoundInProgress = true;
}

void ATeamEliminationMode::AddPlayer(APlayerController* NewPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("AddPlayer called in TeamEliminationMode"));
    if (!NewPlayer)
    {
        return;
    }

    AMyPlayerState* PS = NewPlayer->GetPlayerState<AMyPlayerState>();
    if (!PS) {
		UE_LOG(LogTemp, Warning, TEXT("PlayerState is null in AddPlayer"));
        return;
    }
	UE_LOG(LogTemp, Warning, TEXT("Adding player to team..."));

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

FString ATeamEliminationMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) {

    UE_LOG(LogTemp, Warning, TEXT("InitNewPlayer called in TeamEliminationMode"));
    UE_LOG(LogTemp, Warning, TEXT("DEBUGXXX-01"));
    AddPlayer(NewPlayerController);
	return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);    
}

void ATeamEliminationMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) {
    UE_LOG(LogTemp, Warning, TEXT("HandleStartingNewPlayer_Implementation called in TeamEliminationMode"));
    UE_LOG(LogTemp, Warning, TEXT("DEBUGXXX-03"));
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

void ATeamEliminationMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// log number of players in each team
	UE_LOG(LogTemp, Warning, TEXT("Team A has %d players."), TeamA.Num());
	UE_LOG(LogTemp, Warning, TEXT("Team B has %d players."), TeamB.Num());
}

AActor* ATeamEliminationMode::ChoosePlayerStart_Implementation(AController* Player)
{
    UE_LOG(LogTemp, Warning, TEXT("DEBUGXXX-02"));
    AMyPlayerState* PS = Player->GetPlayerState<AMyPlayerState>();


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

void ATeamEliminationMode::NotifyPlayerKilled(class AController* Killer, class AController* Victim, class UWeaponData* DamageCauser, bool bWasHeadShot)
{
    Super::NotifyPlayerKilled(Killer, Victim, DamageCauser);
	UE_LOG(LogTemp, Warning, TEXT("NotifyPlayerKilled called in TeamEliminationMode"));

	// log damage causer is null or not
	UE_LOG(LogTemp, Warning, TEXT("DamageCauser is %snull"), DamageCauser ? TEXT("not ") : TEXT(""));
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

    ATeamEliminationState* GS = GetGameState<ATeamEliminationState>();
    if (GS)
    {
        GS->MulticastKillNotify(KillerPS, VictimPS, DamageCauser, bWasHeadShot);
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

