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
class FPSDEMO_API AShooterGameState : public AGameState
{
    GENERATED_BODY()

protected:
	UPROPERTY(ReplicatedUsing = OnRep_MyMatchState)
	EMyMatchState CurrentMatchState;

    UPROPERTY(ReplicatedUsing = OnRep_Score)
    int TeamAScore = 0;

    UPROPERTY(ReplicatedUsing = OnRep_Score)
    int TeamBScore = 0;

    UPROPERTY(ReplicatedUsing = OnRep_RoundEndTime)
    int RoundEndTime = -1;

    UPROPERTY(ReplicatedUsing = OnRep_BuyEndTime)
    int BuyEndTime = -1;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentRound)
    int CurrentRound = -1;

    UPROPERTY(ReplicatedUsing = OnRep_HeroPhase) // zombie mode
	bool bHeroPhase = false;

    UPROPERTY(ReplicatedUsing = OnRep_RemainingHeroCount) // zombie mode
    int RemainingHeroCount;

    UPROPERTY(ReplicatedUsing = OnRep_RemainingZombieCount) // zombie mode
    int RemainingZombieCount;

	UPROPERTY(ReplicatedUsing = OnRep_Spike)
    ASpike* PlantedSpike;

    UPROPERTY() // zombie mode, Airdrop Crates
	TArray<AAirdropCrate*> ActiveAirdropCrates;

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

	bool AreSlotsReady() const;
    void TryBroadcastSlotsReady();

    virtual void RemovePlayerState(APlayerState* PlayerState) override;
    virtual void AddPlayerState(APlayerState* PlayerState) override;

    UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnAirdropCrate(FVector Location);

    UFUNCTION(NetMulticast, Unreliable)
	void Multicast_ClaimAirdropCrate(ABaseCharacter* Claimer, EItemId GiftId);
public:
	AShooterGameState();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastKillNotify(AMyPlayerState* Killer, AMyPlayerState* Victim, const UItemConfig* DamageCauser, bool bWasHeadShot);
    UFUNCTION(NetMulticast, Reliable)
	void Multicast_GameResult(ETeamId WinningTeam);
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_SwitchSide();
    void SetMatchState(EMyMatchState NewState);
    EMyMatchState GetMatchState() const {
		return CurrentMatchState;
	}
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RoundResult(ETeamId WinningTeam);

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

    void AddScoreTeam(ETeamId TeamId, int ScoreToAdd);
    int GetScoreTeam(ETeamId TeamId) const;

    void SetRoundEndTime(int NewRoundEndTime);
    int GetRoundEndTime() const {
        return RoundEndTime;
    }
    void SetRoundRemainingTime(int TimeRemaining);
    void SetMatchMode(EMatchMode NewMode);
	EMatchMode GetMatchMode() const { return MatchMode; }
    void SetBuyEndTime(int NewBuyEndTime) {
        BuyEndTime = NewBuyEndTime;
	}
    int GetBuyEndTime() const {
        return BuyEndTime;
	}
    int GetTeamAScore() const { // always score of attackers/soldiers team
		return TeamAScore;
	}
	int GetTeamBScore() const {
		return TeamBScore;
	}
    void SetTeamAScore(int NewScore) {
		TeamAScore = NewScore;
	}
    void SetTeamBScore(int NewScore) {
        TeamBScore = NewScore;
    }
    int GetCurrentRound() const {
        return CurrentRound;
    }
    void SetCurrentRound(int NewRound) {
        CurrentRound = NewRound;
	}

    float GetRemainingRoundTime() const;
    void SetPlantedSpike(ASpike* NewSpike) {
        PlantedSpike = NewSpike;
    }
    ASpike* GetPlantedSpike() const {
        return PlantedSpike;
	}
    void SetHeroPhase(bool bNewHeroPhase);
  
    void SetRemainingHeroCount(int NewCount);
    void SetRemainingZombieCount(int NewCount);
    int GetRemainingHeroCount() const {
		return RemainingHeroCount;
	}
	int GetRemainingZombieCount() const {
		return RemainingZombieCount;
	}
    bool IsHeroPhase() const {
        return bHeroPhase;
    }
    bool CanQuitMidMatch() const;

    void OnSpawnedAirdropCrate(AAirdropCrate* Crate);
    void OnClaimedAirdropCrate(AAirdropCrate* Crate, ABaseCharacter* Claimer, EItemId GiftId);
	TArray<AAirdropCrate*> GetActiveAirdropCrates() const { return ActiveAirdropCrates; }
	void ClearAirdropCrates() { ActiveAirdropCrates.Empty(); }

    UPROPERTY(ReplicatedUsing=OnRep_PlayerSlots)
    TArray<APlayerSlot*> Slots;

	APlayerSlot* GetPlayerSlot(int32 PlayerId);

    FTimerHandle SlotsRetryHandle;
    int32 SlotsRetryCount = 0;
};
