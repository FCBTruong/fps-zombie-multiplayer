// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ShooterGameState.h"
#include "Net/UnrealNetwork.h"
#include "Pickup/PickupItem.h"
#include "Controllers/MyPlayerController.h"
#include "Controllers/MyPlayerState.h"
#include "Items/ItemConfig.h"
#include "UI/PlayerUI.h"
#include "Spike/Spike.h"
#include "Game/GameManager.h"
#include "Kismet/GameplayStatics.h"
#include "Game/GlobalDataAsset.h"

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
	DOREPLIFETIME(AShooterGameState, RoundEndTime);
	DOREPLIFETIME(AShooterGameState, CurrentMatchState);
    DOREPLIFETIME(AShooterGameState, MatchMode);
	DOREPLIFETIME(AShooterGameState, BuyEndTime);
	DOREPLIFETIME(AShooterGameState, CurrentRound);
	DOREPLIFETIME(AShooterGameState, PlantedSpike);
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
        if (!MyPC->GetPlayerUI()) {
            return;
		}   

		MyPC->GetPlayerUI()->NotifyKill(Killer, Victim, DamageCauser, bWasHeadShot);

		AActor* KillerPawn = Killer ? Killer->GetPawn() : nullptr;
        AActor* ViewTargetPawn = MyPC->GetViewTarget();

        if (KillerPawn && KillerPawn == ViewTargetPawn)
        {
			MyPC->GetPlayerUI()->ShowKillMark(bWasHeadShot);
		}
    }
}

void AShooterGameState::Multicast_RoundResult_Implementation(ETeamId WinningTeam)
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        AMyPlayerController* MyPC = Cast<AMyPlayerController>(It->Get());
        if (MyPC && MyPC->GetPlayerUI())
        {
            FText ResultText;
            if (WinningTeam == ETeamId::Soldier) {
                ResultText = FText::FromString("Solider Win");
            }
            else if (WinningTeam == ETeamId::Zombie) {
                ResultText = FText::FromString("Zombie Win");
            }
            else {
                bool IsWinner = false;
                if (MyPC->GetTeamId() == WinningTeam) {
                    IsWinner = true;
                }

                // play sound
                if (UGameManager* GM = UGameManager::Get(GetWorld()))
                {
                    if (GM->GlobalData && GM->GlobalData->RoundEndSound)
                    {
                        UGameplayStatics::PlaySound2D(GetWorld(), GM->GlobalData->RoundEndSound.Get());
                    }
                }
                ResultText = IsWinner ? FText::FromString("ROUND WIN") : FText::FromString("ROUND LOSE");
            }
			MyPC->GetPlayerUI()->ShowMatchStateToast(ResultText, 0);
        }
    }
}

void AShooterGameState::AddScoreTeam(ETeamId TeamId, int ScoreToAdd)
{
    if (TeamId == ETeamId::Attacker)
    {
        TeamAScore += ScoreToAdd;
    }
    else if (TeamId == ETeamId::Defender)
    {
        TeamBScore += ScoreToAdd;
    }
}

int AShooterGameState::GetScoreTeam(ETeamId TeamId) const
{
    if (TeamId == ETeamId::Attacker)
    {
        return TeamAScore;
    }
    else if (TeamId == ETeamId::Defender)
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

void AShooterGameState::OnRep_BuyEndTime()
{
    // You can add any client-side logic here that needs to respond to BuyEndTime changes
}

void AShooterGameState::SetMatchState(EMyMatchState NewState)
{
    if (!HasAuthority()) return;
    CurrentMatchState = NewState;
    OnRep_MyMatchState(); // apply immediately on server
}

void AShooterGameState::AddPlayerState(APlayerState* PlayerState)
{
    Super::AddPlayerState(PlayerState);

    OnPlayerAdded.Broadcast(PlayerState);

    UE_LOG(LogTemp, Warning,
        TEXT("AddPlayerState | HasAuthority=%d | NetMode=%d"),
        HasAuthority(),
        GetNetMode()
    );
}

void AShooterGameState::RemovePlayerState(APlayerState* PlayerState)
{
    Super::RemovePlayerState(PlayerState);

    OnPlayerRemoved.Broadcast(PlayerState);

    UE_LOG(LogTemp, Warning,
        TEXT("RemovePlayerState | HasAuthority=%d | NetMode=%d"),
        HasAuthority(),
        GetNetMode()
    );
}

float AShooterGameState::GetRemainingRoundTime() const
{
    if (RoundEndTime < 0) {
        return 0.f;
    }
    int32 CurrentTime = GetWorld()->GetTimeSeconds();
    int32 RemainingTime = RoundEndTime - CurrentTime;
    return FMath::Max(0, RemainingTime);
}

void AShooterGameState::OnRep_CurrentRound()
{
    
}

void AShooterGameState::OnRep_Spike()
{
    
}

void AShooterGameState::Multicast_GameResult_Implementation(ETeamId WinningTeam)
{
	OnGameResult.Broadcast(WinningTeam);
}

void AShooterGameState::Multicast_SwitchSide_Implementation()
{
	OnSwitchSide.Broadcast();
}