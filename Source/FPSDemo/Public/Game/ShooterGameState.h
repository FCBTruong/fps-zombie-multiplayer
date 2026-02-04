#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Pickup/PickupData.h"
#include "Controllers/MyPlayerState.h"
#include "Game/MyMatchState.h"
#include "Data/MatchMode.h"
#include "ShooterGameState.generated.h"

class ASpike;
class AAirdropCrate;
class ABaseCharacter;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUpdateScore, int32, int32)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateRoundTime, int32)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateMatchState, const EMyMatchState&)
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

    UPROPERTY(ReplicatedUsing = OnRep_MatchMode)
    EMatchMode MatchMode = EMatchMode::Spike;

    UFUNCTION()
    void OnRep_MatchMode();

    UFUNCTION()
	void OnRep_BuyEndTime();

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

    void AddScoreTeam(ETeamId TeamId, int ScoreToAdd);
    int GetScoreTeam(ETeamId TeamId) const;

    void SetRoundEndTime(int NewRoundEndTime) {
        RoundEndTime = NewRoundEndTime;
	}
    int GetRoundEndTime() const {
        return RoundEndTime;
    }
    void SetRoundRemainingTime(int TimeRemaining) {
        RoundEndTime = GetWorld()->GetTimeSeconds() + TimeRemaining;
    }   
    void SetMatchMode(EMatchMode NewMode);
	EMatchMode GetMatchMode() const { return MatchMode; }
    void SetBuyEndTime(int NewBuyEndTime) {
        BuyEndTime = NewBuyEndTime;
	}
    int GetBuyEndTime() const {
        return BuyEndTime;
	}
    int GetTeamAScore() const {
		return TeamAScore;
	}
	int GetTeamBScore() const {
		return TeamBScore;
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
    void SetHeroPhase(bool bNewHeroPhase) {
        bHeroPhase = bNewHeroPhase;
    }
    bool IsHeroPhase() const {
        return bHeroPhase;
    }
    bool CanQuitMidMatch() const;

    void OnSpawnedAirdropCrate(AAirdropCrate* Crate);
    void OnClaimedAirdropCrate(AAirdropCrate* Crate, ABaseCharacter* Claimer, EItemId GiftId);
	TArray<AAirdropCrate*> GetActiveAirdropCrates() const { return ActiveAirdropCrates; }
	void ClearAirdropCrates() { ActiveAirdropCrates.Empty(); }
};
