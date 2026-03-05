// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Framework/ShooterGameState.h"
#include "Net/UnrealNetwork.h"
#include "Game/Items/Pickup/PickupItem.h"
#include "Game/Framework/MyPlayerController.h"
#include "Game/Framework/MyPlayerState.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "Game/UI/PlayerUI.h"
#include "Game/Modes/Spike/Spike.h"
#include "Game/GameManager.h"
#include "Kismet/GameplayStatics.h"
#include "Shared/Data/GlobalDataAsset.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/Modes/Zombie/AirdropCrate.h"
#include "Shared/System/ItemsManager.h"
#include "Game/Framework/PlayerSlot.h"
#include "Game/Modes/Zombie/HealthPack.h"

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
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC);
    if (!MyPC || !MyPC->GetPlayerUI()) {
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

void AShooterGameState::Multicast_RoundResult_Implementation(ETeamId WinningTeam)
{
	AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
    if (!MyPC || !MyPC->GetPlayerUI()) {
        return;
    }
    UGameManager* GM = UGameManager::Get(GetWorld());
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
        const bool bIsWinner = MyPC->GetTeamId() == WinningTeam;
        UGameplayStatics::PlaySound2D(GetWorld(), GM->GlobalData->RoundEndSound.Get());
        ResultText = bIsWinner ? FText::FromString("ROUND WIN") : FText::FromString("ROUND LOSE");
    }
	MyPC->GetPlayerUI()->ShowMatchStateToast(ResultText, 0);
}

void AShooterGameState::AddScoreTeam(ETeamId TeamId, int ScoreToAdd)
{
    if (TeamId == ETeamId::Attacker || TeamId == ETeamId::Soldier)
    {
        TeamAScore += ScoreToAdd;
    }
    else if (TeamId == ETeamId::Defender || TeamId == ETeamId::Zombie)
    {
        TeamBScore += ScoreToAdd;
    }

	OnRep_Score(); // update immediately on server
}

int AShooterGameState::GetScoreTeam(ETeamId TeamId) const
{
	if (TeamId == ETeamId::Attacker || TeamId == ETeamId::Soldier)
    {
        return TeamAScore;
    }
    else if (TeamId == ETeamId::Defender || TeamId == ETeamId::Zombie)
    {
        return TeamBScore;
    }
    return 0;
}

void AShooterGameState::OnRep_Score()
{
    OnUpdateScore.Broadcast(TeamAScore, TeamBScore);
}

void AShooterGameState::OnRep_RoundEndTime()
{
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
}

void AShooterGameState::RemovePlayerState(APlayerState* PlayerState)
{
    Super::RemovePlayerState(PlayerState);

    OnPlayerRemoved.Broadcast(PlayerState);
}

float AShooterGameState::GetRemainingRoundTime() const
{
    if (RoundEndTime < 0) {
        return 0.f;
    }
    float Now = 0;
    if (const UWorld* World = GetWorld())
    {
        if (const AGameStateBase* GS = World->GetGameState())
        {
            Now = GS->GetServerWorldTimeSeconds();
        }
    }
    int32 RemainingTime = RoundEndTime - Now;
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
    TWeakObjectPtr<AShooterGameState> WeakThis(this);

    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,
        [WeakThis]()
        {
            if (!WeakThis.IsValid())
            {
                return;
            }

            UWorld* World = WeakThis->GetWorld();
            if (!World)
            {
                return;
            }

            if (AGameModeBase* GM = World->GetAuthGameMode<AGameModeBase>())
            {
                GM->ReturnToMainMenuHost();
            }
        },
        5.f,
        false
    );
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
    return false;
}

void AShooterGameState::OnSpawnedHealthPack(AHealthPack* NewPack)
{
    if (!HasAuthority() || !NewPack) return;
    ActiveHealthPacks.Add(NewPack);
}

void AShooterGameState::OnClaimedHealthPack(AHealthPack* ClaimedPack)
{
    if (!HasAuthority() || !ClaimedPack) return;
    ActiveHealthPacks.Remove(ClaimedPack);
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
    if (!MyPC) {
        return;
    }
    MyPC->NotifyToastMessage(FText::FromString("Airdrop crate has arrived!"));

    UGameManager* GM = UGameManager::Get(GetWorld());
    if (GM->GlobalData->AirdropCrateSpawnSound)
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

APlayerSlot* AShooterGameState::GetPlayerSlot(int32 PlayerId) const {
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

    constexpr int32 MaxSlotsRetry = 5;
    if (SlotsRetryCount++ >= MaxSlotsRetry)
    {
        GetWorldTimerManager().ClearTimer(SlotsRetryHandle);
    }
}

bool AShooterGameState::AreSlotsReady() const
{
    for (APlayerSlot* S : Slots)
    {
        if (!IsValid(S)) return false;              // NetGUID not resolved yet
        if (!S->HasActorBegunPlay()) {
            return false;  // not initialized yet
        }
    }
    return true;
}

void AShooterGameState::SetRoundRemainingTime(int TimeRemaining) {
    float Now = 0;
    if (const UWorld* World = GetWorld())
    {
        if (const AGameStateBase* GS = World->GetGameState())
        {
            Now = GS->GetServerWorldTimeSeconds();
        }
    }
    int TimeEnd = FMath::CeilToInt(Now + TimeRemaining);
	SetRoundEndTime(TimeEnd);
}

void AShooterGameState::SetRoundEndTime(int32 TimeEnd) {
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

void AShooterGameState::SetTeamAScore(int NewScore) {
    if (!HasAuthority()) return;
    TeamAScore = NewScore;
    OnRep_Score(); // apply immediately on server too 
}

void AShooterGameState::SetTeamBScore(int NewScore) {
    if (!HasAuthority()) return;
    TeamBScore = NewScore;
    OnRep_Score(); // apply immediately on server too 
}

void AShooterGameState::SetCurrentRound(int NewRound) {
    if (!HasAuthority()) return;
    CurrentRound = NewRound;
    OnRep_CurrentRound(); // apply immediately on server too 
}

void AShooterGameState::SetPlantedSpike(ASpike* NewSpike) {
    if (!HasAuthority()) return;
    PlantedSpike = NewSpike;
    OnRep_Spike(); // apply immediately on server too 
}

void AShooterGameState::SetBuyEndTime(int NewBuyEndTime){
    BuyEndTime = NewBuyEndTime;
}

void AShooterGameState::ClearActiveItems() {
    ActiveAirdropCrates.Empty();
    ActiveHealthPacks.Empty();
}

int AShooterGameState::GetRoundEndTime() const {
    return RoundEndTime;
}

int AShooterGameState::GetBuyEndTime() const {
    return BuyEndTime;
}

int AShooterGameState::GetTeamAScore() const { // always score of attackers/soldiers team
    return TeamAScore;
}

int AShooterGameState::GetTeamBScore() const {
    return TeamBScore;
}

int AShooterGameState::GetCurrentRound() const {
    return CurrentRound;
}

ASpike* AShooterGameState::GetPlantedSpike() const {
    return PlantedSpike;
}

int AShooterGameState::GetRemainingHeroCount() const {
    return RemainingHeroCount;
}

int AShooterGameState::GetRemainingZombieCount() const {
    return RemainingZombieCount;
}

bool AShooterGameState::IsHeroPhase() const {
    return bHeroPhase;
}

EMatchMode AShooterGameState::GetMatchMode() const { 
    return MatchMode; 
}

const TArray<AAirdropCrate*>&  AShooterGameState::GetActiveAirdropCrates() const {
    return ActiveAirdropCrates; 
}

const TArray<AHealthPack*>& AShooterGameState::GetActiveHealthPacks() const {
    return ActiveHealthPacks;
}

EMyMatchState AShooterGameState::GetMatchState() const {
	return CurrentMatchState;
}

void AShooterGameState::Multicast_RoundStart_Implementation() {
    UGameManager* GMR = UGameManager::Get(GetWorld());
    if (MatchMode != EMatchMode::Spike) {
        return;
    }
    if (GMR->GlobalData->RoundStartSound) {
        UGameplayStatics::PlaySound2D(GetWorld(), GMR->GlobalData->RoundStartSound.Get());
    }
}