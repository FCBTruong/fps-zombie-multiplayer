// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ShooterGameState.h"
#include <Net/UnrealNetwork.h>
#include "Pickup/PickupItem.h"
#include "Controllers/MyPlayerController.h"
#include "Controllers/MyPlayerState.h"
#include "Items/ItemConfig.h"

AShooterGameState::AShooterGameState()
{
    // Enable replication
    bReplicates = true;
}

void AShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterGameState, TeamAScore);
	DOREPLIFETIME(AShooterGameState, TeamBScore);
	DOREPLIFETIME(AShooterGameState, AttackerTeam);
	DOREPLIFETIME(AShooterGameState, RoundEndTime);
	DOREPLIFETIME(AShooterGameState, CurrentMatchState);
    DOREPLIFETIME(AShooterGameState, MatchMode);
}


void AShooterGameState::MulticastKillNotify_Implementation(AMyPlayerState* Killer, AMyPlayerState* Victim, const UItemConfig* DamageCauser, bool bWasHeadShot)
{
    UE_LOG(LogTemp, Warning,
        TEXT("MulticastKillNotify called | Killer=%s | Victim=%s | Weapon=%s | Headshot=%d"),
        IsValid(Killer) ? *Killer->GetPathName() : TEXT("NULL"),
        IsValid(Victim) ? *Victim->GetPathName() : TEXT("NULL"),
        IsValid(DamageCauser) ? *DamageCauser->GetPathName() : TEXT("NULL"),
        bWasHeadShot
    );
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC);
        if (!MyPC) {
            return;
        }
        if (!MyPC->PlayerUI) {
            return;
		}   

		MyPC->PlayerUI->NotifyKill(Killer, Victim, DamageCauser, bWasHeadShot);
    }
}

void AShooterGameState::Multicast_RoundResult_Implementation(FName WinningTeam)
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        AMyPlayerController* MyPC = Cast<AMyPlayerController>(It->Get());
        if (MyPC && MyPC->PlayerUI)
        {
			bool IsWinner = false;
            if (MyPC->GetTeamId() == WinningTeam) {
				IsWinner = true;
            }
			FText ResultText = IsWinner ? FText::FromString("ROUND WIN") : FText::FromString("ROUND LOSE");
			MyPC->PlayerUI->ShowMatchStateToast(ResultText, 1);
        }
    }
}

void AShooterGameState::AddScoreTeam(FName TeamId, int ScoreToAdd)
{
    if (TeamId == FName(TEXT("A")))
    {
        TeamAScore += ScoreToAdd;
    }
    else if (TeamId == FName(TEXT("B")))
    {
        TeamBScore += ScoreToAdd;
    }
}

int AShooterGameState::GetScoreTeam(FName TeamId) const
{
    if (TeamId == FName(TEXT("A")))
    {
        return TeamAScore;
    }
    else if (TeamId == FName(TEXT("B")))
    {
        return TeamBScore;
    }
    return 0;
}

void AShooterGameState::OnRep_Score()
{
	UE_LOG(LogTemp, Log, TEXT("Scores updated: Team A: %d, Team B: %d"), TeamAScore, TeamBScore);
    OnUpdateScore.Broadcast(TeamAScore, TeamBScore);
}

void AShooterGameState::OnRep_RoundEndTime()
{
    // You can add any client-side logic here that needs to respond to RoundEndTime changes
    if (RoundEndTime < 0) {
        //return;
	}
	OnUpdateRoundTime.Broadcast(RoundEndTime);
}

void AShooterGameState::OnRep_MyMatchState()
{
    OnUpdateMatchState.Broadcast(CurrentMatchState);
}

void AShooterGameState::SetMatchMode(EMatchMode NewMode)
{
    if (!HasAuthority()) return;
    if (MatchMode == NewMode) return;

    MatchMode = NewMode;
    OnRep_MatchMode(); // apply immediately on server too 
}

void AShooterGameState::OnRep_MatchMode()
{
    // optional: broadcast event if you want pawns to re-apply visuals
}