#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Game/Items/Pickup/PickupData.h"
#include "Game/Framework/MyPlayerState.h"
#include "Game/Types/MyMatchState.h"
#include "Shared/Types/MatchMode.h"
#include "ShooterGameState.generated.h"

class ASpike;
class AAirdropCrate;
class AHealthPack;
class ABaseCharacter;
class APlayerSlot;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUpdateScore, int32, int32)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateRoundTime, int32)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateMatchState, EMyMatchState)
DECLARE_MULTICAST_DELEGATE_OneParam(
    FOnPlayerStateAdded,
    APlayerState*
);

DECLARE_MULTICAST_DELEGATE_OneParam(
    FOnPlayerStateRemoved,
    APlayerState*
);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameResult, ETeamId);
DECLARE_MULTICAST_DELEGATE(FOnSwitchSide);
DECLARE_MULTICAST_DELEGATE(FOnUpdateRoundNumber);
DECLARE_MULTICAST_DELEGATE(FOnUpdateHeroPhase); // for zombie mode, refactor later
DECLARE_MULTICAST_DELEGATE(FOnUpdateHeroZombieCount);
DECLARE_MULTICAST_DELEGATE(FOnUpdatePlayerSlots);

UCLASS()
class FPSDEMO_API AShooterGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AShooterGameState();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void RemovePlayerState(APlayerState* PlayerState) override;
    virtual void AddPlayerState(APlayerState* PlayerState) override;

	// Multicast RPCs
    UFUNCTION(NetMulticast, Reliable)
    void MulticastKillNotify(AMyPlayerState* Killer, AMyPlayerState* Victim, const UItemConfig* DamageCauser, bool bWasHeadShot);
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_GameResult(ETeamId WinningTeam);
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_SwitchSide();
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_RoundResult(ETeamId WinningTeam);
	UFUNCTION(NetMulticast, Unreliable) // only used for visual/audio effect, so unreliable is fine
    void Multicast_RoundStart();

	// Setters and Getters
    void SetMatchState(EMyMatchState NewState);
    void SetBuyEndTime(int NewBuyEndTime);
    void AddScoreTeam(ETeamId TeamId, int ScoreToAdd);
    void SetRoundEndTime(int32 NewRoundEndTime);
    void SetRoundRemainingTime(int TimeRemaining);
    void SetMatchMode(EMatchMode NewMode);
    void SetTeamAScore(int NewScore);
    void SetTeamBScore(int NewScore);
    void SetPlantedSpike(ASpike* NewSpike);
    void SetCurrentRound(int NewRound);
    void SetHeroPhase(bool bNewHeroPhase);
    void SetRemainingHeroCount(int NewCount);
    void SetRemainingZombieCount(int NewCount);
    void OnSpawnedAirdropCrate(AAirdropCrate* Crate);
	void OnSpawnedHealthPack(AHealthPack* HealthPack);
	void OnClaimedHealthPack(AHealthPack* HealthPack);
    void OnClaimedAirdropCrate(AAirdropCrate* Crate, ABaseCharacter* Claimer, EItemId GiftId);
	void ClearActiveItems();
    int GetRoundEndTime() const;
    int GetScoreTeam(ETeamId TeamId) const;
    int GetBuyEndTime() const;
    int GetTeamAScore() const;
    int GetTeamBScore() const;
    int GetCurrentRound() const;
    int GetRemainingHeroCount() const;
    int GetRemainingZombieCount() const;
    bool IsHeroPhase() const;
    bool CanQuitMidMatch() const;
    bool AreSlotsReady() const;
    float GetRemainingRoundTime() const;
    ASpike* GetPlantedSpike() const;
    EMyMatchState GetMatchState() const;
    EMatchMode GetMatchMode() const;
    APlayerSlot* GetPlayerSlot(int32 PlayerId) const;
    const TArray<AAirdropCrate*>& GetActiveAirdropCrates() const;
    const TArray<AHealthPack*>& GetActiveHealthPacks() const;

    UPROPERTY(ReplicatedUsing = OnRep_PlayerSlots)
    TArray<APlayerSlot*> Slots;

    FTimerHandle SlotsRetryHandle;
    int32 SlotsRetryCount = 0;
public:
	// Delegates
    FOnUpdateScore OnUpdateScore;
    FOnUpdateRoundTime OnUpdateRoundTime;
    FOnUpdateMatchState OnUpdateMatchState;
    FOnPlayerStateAdded   OnPlayerAdded;
    FOnPlayerStateRemoved OnPlayerRemoved;
    FOnGameResult OnGameResult;
    FOnSwitchSide OnSwitchSide;
    FOnUpdateRoundNumber OnUpdateRoundNumber;
    FOnUpdateHeroPhase OnUpdateHeroPhase;
    FOnUpdateHeroZombieCount OnUpdateHeroZombieCount;
    FOnUpdatePlayerSlots OnUpdatePlayerSlots;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_MyMatchState)
	EMyMatchState CurrentMatchState;

    UPROPERTY(ReplicatedUsing = OnRep_Score)
    int32 TeamAScore = 0;

    UPROPERTY(ReplicatedUsing = OnRep_Score)
    int32 TeamBScore = 0;

    UPROPERTY(ReplicatedUsing = OnRep_RoundEndTime)
    int32 RoundEndTime = -1;

    UPROPERTY(ReplicatedUsing = OnRep_BuyEndTime)
    int32 BuyEndTime = -1;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentRound)
    int32 CurrentRound = -1;

    UPROPERTY(ReplicatedUsing = OnRep_HeroPhase) // zombie mode
	bool bHeroPhase = false;

    UPROPERTY(ReplicatedUsing = OnRep_RemainingHeroCount) // zombie mode
    int32 RemainingHeroCount;

    UPROPERTY(ReplicatedUsing = OnRep_RemainingZombieCount) // zombie mode
    int32 RemainingZombieCount;

	UPROPERTY(ReplicatedUsing = OnRep_Spike)
    ASpike* PlantedSpike;

    UPROPERTY() // zombie mode, Airdrop Crates
	TArray<AAirdropCrate*> ActiveAirdropCrates;

    UPROPERTY() // zombie mode, HealthPacks
    TArray<AHealthPack*> ActiveHealthPacks;

    UFUNCTION()
	void OnRep_Spike();
    
    UFUNCTION()
    void OnRep_Score();

	UFUNCTION()
	void OnRep_RoundEndTime();

    UFUNCTION()
	void OnRep_CurrentRound();

	UFUNCTION()
	void OnRep_MyMatchState();

    UFUNCTION()
	void OnRep_HeroPhase();

	UFUNCTION()
	void OnRep_RemainingHeroCount();

	UFUNCTION()
	void OnRep_RemainingZombieCount();

    UPROPERTY(ReplicatedUsing = OnRep_MatchMode)
    EMatchMode MatchMode = EMatchMode::Spike;

    UFUNCTION()
    void OnRep_MatchMode();

    UFUNCTION()
	void OnRep_BuyEndTime();

    UFUNCTION()
	void OnRep_PlayerSlots();

    UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnAirdropCrate(FVector Location);

    UFUNCTION(NetMulticast, Unreliable)
	void Multicast_ClaimAirdropCrate(ABaseCharacter* Claimer, EItemId GiftId);

    void TryBroadcastSlotsReady();
};
