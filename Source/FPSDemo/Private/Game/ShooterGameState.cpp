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
#include "Characters/BaseCharacter.h"
#include "Items/AirdropCrate.h"
#include "Game/ItemsManager.h"
#include "Game/PlayerSlot.h"

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
	DOREPLIFETIME(AShooterGameState, bHeroPhase);
	DOREPLIFETIME(AShooterGameState, RemainingHeroCount);
	DOREPLIFETIME(AShooterGameState, RemainingZombieCount);
	DOREPLIFETIME(AShooterGameState, Slots);
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
	AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
    if (!MyPC || !MyPC->GetPlayerUI()) {
        return;
    }
    UGameManager* GM = UGameManager::Get(GetWorld());
    if (!GM || !GM->GlobalData) {
        return;
	}
    FText ResultText;
    if (WinningTeam == ETeamId::Soldier) {
        ResultText = FText::FromString("Soldiers Win");
        UGameplayStatics::PlaySound2D(GetWorld(), GM->GlobalData->SoldierWinVoice.Get());
    }
    else if (WinningTeam == ETeamId::Zombie) {
        ResultText = FText::FromString("Zombies Win");
        UGameplayStatics::PlaySound2D(GetWorld(), GM->GlobalData->ZombieWinVoice.Get());
    }
    else {
        bool IsWinner = false;
        if (MyPC->GetTeamId() == WinningTeam) {
            IsWinner = true;
        }

        UGameplayStatics::PlaySound2D(GetWorld(), GM->GlobalData->RoundEndSound.Get());
          
        ResultText = IsWinner ? FText::FromString("ROUND WIN") : FText::FromString("ROUND LOSE");
    }
	MyPC->GetPlayerUI()->ShowMatchStateToast(ResultText, 0);
}

void AShooterGameState::AddScoreTeam(ETeamId TeamId, int ScoreToAdd)
{
    if (TeamId == ETeamId::Attacker || 
		TeamId == ETeamId::Soldier
        )
    {
        TeamAScore += ScoreToAdd;
    }
    else if (TeamId == ETeamId::Defender
		|| TeamId == ETeamId::Zombie
        )
    {
        TeamBScore += ScoreToAdd;
    }

	OnRep_Score(); // update immediately on server
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
	OnUpdateRoundNumber.Broadcast();
}

void AShooterGameState::OnRep_Spike()
{
    
}

void AShooterGameState::Multicast_GameResult_Implementation(ETeamId WinningTeam)
{
	OnGameResult.Broadcast(WinningTeam);

	// back to lobby after 5 seconds
	FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]() {
        if (AGameModeBase* GM = GetWorld()->GetAuthGameMode<AGameModeBase>()) {
            GM->ReturnToMainMenuHost();
        }
		}, 5.f, false);
}

void AShooterGameState::Multicast_SwitchSide_Implementation()
{
	OnSwitchSide.Broadcast();
}

void AShooterGameState::OnRep_HeroPhase()
{
    OnUpdateHeroPhase.Broadcast();
}

bool AShooterGameState::CanQuitMidMatch() const
{
    // Prevent quitting mid-match during critical phases
    if (MatchMode == EMatchMode::DeathMatch)
    {
        return true; // Allow quitting in DeathMatch mode
	}
    return false;
}

void AShooterGameState::OnSpawnedAirdropCrate(AAirdropCrate* NewCrate)
{
    if (!HasAuthority() || !NewCrate) return;
    ActiveAirdropCrates.Add(NewCrate);

	FVector Location = NewCrate->GetActorLocation();
    Multicast_SpawnAirdropCrate(Location);
}

void AShooterGameState::Multicast_SpawnAirdropCrate_Implementation(FVector Location)
{
	// dedicated then return
    if (GetNetMode() == NM_DedicatedServer) {
        return;
    }
    // Clients can play spawn effects here if needed
	AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
	MyPC->NotifyToastMessage(FText::FromString("Airdrop crate has arrived!"));

    if (UGameManager* GM = UGameManager::Get(GetWorld()))
    {
        if (GM->GlobalData && GM->GlobalData->AirdropCrateSpawnSound)
        {
            UGameplayStatics::PlaySound2D(GetWorld(), GM->GlobalData->AirdropCrateSpawnSound.Get());
        }

        if (GM->GlobalData->HelicopterClass) {
			AActor* Helicopter = GetWorld()->SpawnActor<AActor>(GM->GlobalData->HelicopterClass);
            Location.Z += 2000.f; // raise it up
            Helicopter->SetActorLocation(Location);

            // random direction
            FRotator NewRot = FRotator::ZeroRotator;
            NewRot.Yaw = FMath::FRandRange(0.f, 360.f);
            Helicopter->SetActorRotation(NewRot);
			Helicopter->SetLifeSpan(25.f);
            // move forward, and hide after 5 seconds
        }
    }
}

void AShooterGameState::OnClaimedAirdropCrate(AAirdropCrate* ClaimedCrate, ABaseCharacter* ClaimingCharacter, EItemId GiftId)
{
    if (!HasAuthority() || !ClaimedCrate) return;
    ActiveAirdropCrates.Remove(ClaimedCrate);

	Multicast_ClaimAirdropCrate(ClaimingCharacter, GiftId);
}

void AShooterGameState::Multicast_ClaimAirdropCrate_Implementation(ABaseCharacter* Claimer, EItemId GiftId)
{
    // Clients can play claim effects here if needed
    if (Claimer && Claimer->IsLocallyControlled())
    {
        AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
        FText Txt = FText::FromString("Ammo added to primary weapon!");
        if (GiftId != EItemId::NONE)
        {
			const UItemConfig* GiftConfig = UItemsManager::Get(GetWorld())->GetItemById(GiftId);
            if (GiftConfig)
            {
                Txt = FText::FromString(
                    FString::Printf(TEXT("You received %s!"),
                        *GiftConfig->DisplayName.ToString())
                );
            }
		}
        else {
            // if none, mean ammo
        }
        MyPC->NotifyToastMessage(Txt);
    }
}

void AShooterGameState::OnRep_RemainingHeroCount()
{
	OnUpdateHeroZombieCount.Broadcast();
}

void AShooterGameState::OnRep_RemainingZombieCount()
{
    OnUpdateHeroZombieCount.Broadcast();
}

APlayerSlot* AShooterGameState::GetPlayerSlot(int32 PlayerId) {
    for (APlayerSlot* Slot : Slots) {
        if (Slot->GetBackendUserId() == PlayerId) {
            return Slot;
        }
    }
    return nullptr;
}

void AShooterGameState::OnRep_PlayerSlots()
{
    SlotsRetryCount = 0;
    GetWorldTimerManager().ClearTimer(SlotsRetryHandle);
    if (AreSlotsReady()) {
        OnUpdatePlayerSlots.Broadcast();
		return;
    }

    GetWorldTimerManager().SetTimer(
        SlotsRetryHandle,
        this,
        &AShooterGameState::TryBroadcastSlotsReady,
        0.5f,   // interval
        true    // looping
    );
}

void AShooterGameState::TryBroadcastSlotsReady()
{
    if (AreSlotsReady())
    {
        GetWorldTimerManager().ClearTimer(SlotsRetryHandle);
        OnUpdatePlayerSlots.Broadcast();
        return;
    }

    int MaxSlotsRetry = 5;
    if (SlotsRetryCount++ >= MaxSlotsRetry)
    {
        GetWorldTimerManager().ClearTimer(SlotsRetryHandle);
        // optional log once
        return;
    }
}

bool AShooterGameState::AreSlotsReady() const
{
	UE_LOG(LogTemp, Log, TEXT("DEBUGMMM Checking if player slots are ready..."));
    for (APlayerSlot* S : Slots)
    {
        if (!IsValid(S)) return false;              // NetGUID not resolved yet
        if (!S->HasActorBegunPlay()) {
			UE_LOG(LogTemp, Log, TEXT("DEBUGMMM Player slot %d has not begun play yet."), S->GetBackendUserId());
            return false;  // not initialized yet
        }
    }
	UE_LOG(LogTemp, Log, TEXT("DEBUGMMM All player slots are ready."));
    return true;
}

void AShooterGameState::SetRoundRemainingTime(int TimeRemaining) {
    int TimeEnd = GetWorld()->GetTimeSeconds() + TimeRemaining;
	SetRoundEndTime(TimeEnd);
}

void AShooterGameState::SetRoundEndTime(int TimeEnd) {
    if (!HasAuthority()) return;
    RoundEndTime = TimeEnd;
    OnRep_RoundEndTime(); // apply immediately on server too 
}

void AShooterGameState::SetHeroPhase(bool bInHeroPhase) {
    bHeroPhase = bInHeroPhase;
	OnRep_HeroPhase(); // apply immediately on server too
}

void AShooterGameState::SetRemainingHeroCount(int NewCount) {
    RemainingHeroCount = NewCount;
	OnRep_RemainingHeroCount(); // apply immediately on server too
}
void AShooterGameState::SetRemainingZombieCount(int NewCount) {
    RemainingZombieCount = NewCount;
	OnRep_RemainingZombieCount(); // apply immediately on server too
}