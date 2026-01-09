#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Pickup/PickupData.h"
#include "Controllers/MyPlayerState.h"
#include "Game/MyMatchState.h"
#include "ShooterGameState.generated.h"

UENUM(BlueprintType)
enum class EMatchMode : uint8
{
	None,
    Spike,
    Zombie,
    TeamElimination,
	DeathMatch
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUpdateScore, int32, int32)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateRoundTime, int32)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateMatchState, const EMyMatchState&)

UCLASS()
class FPSDEMO_API AShooterGameState : public AGameState
{
    GENERATED_BODY()

protected:
	UPROPERTY(ReplicatedUsing = OnRep_MyMatchState)
	EMyMatchState CurrentMatchState;

	UPROPERTY(Replicated)
    FName AttackerTeam = FName(TEXT("A"));

    UPROPERTY(ReplicatedUsing = OnRep_Score)
    int TeamAScore = 0;

    UPROPERTY(ReplicatedUsing = OnRep_Score)
    int TeamBScore = 0;

    UPROPERTY(ReplicatedUsing = OnRep_RoundEndTime)
    int RoundEndTime = -1;

    UPROPERTY(ReplicatedUsing = OnRep_BuyEndTime)
    int BuyEndTime = -1;
    
    UFUNCTION()
    void OnRep_Score();

	UFUNCTION()
	void OnRep_RoundEndTime();

	UFUNCTION()
	void OnRep_MyMatchState();

    UPROPERTY(ReplicatedUsing = OnRep_MatchMode)
    EMatchMode MatchMode = EMatchMode::Spike;

    UFUNCTION()
    void OnRep_MatchMode();

    UFUNCTION()
	void OnRep_BuyEndTime();
public:
	AShooterGameState();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastKillNotify(AMyPlayerState* Killer, AMyPlayerState* Victim, const UItemConfig* DamageCauser, bool bWasHeadShot);
    void SetMatchState(EMyMatchState NewState);
    EMyMatchState GetMatchState() const {
		return CurrentMatchState;
	}
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RoundResult(FName WinningTeam);
    FName GetAttackerTeam() const {
        return AttackerTeam;
	}
    FName GetDefenderTeam() const {
        return AttackerTeam == FName(TEXT("A")) ? FName(TEXT("B")) : FName(TEXT("A"));
	}

    FOnUpdateScore OnUpdateScore;
	FOnUpdateRoundTime OnUpdateRoundTime;
	FOnUpdateMatchState OnUpdateMatchState;

    void AddScoreTeam(FName TeamId, int ScoreToAdd);
    int GetScoreTeam(FName TeamId) const;
    void SetAttackerTeam(FName NewAttackerTeam) {
        AttackerTeam = NewAttackerTeam;
    }
    void SetRoundEndTime(int NewRoundEndTime) {
        RoundEndTime = NewRoundEndTime;
	}
    void SetMatchMode(EMatchMode NewMode);
	EMatchMode GetMatchMode() const { return MatchMode; }
    void SetBuyEndTime(int NewBuyEndTime) {
        BuyEndTime = NewBuyEndTime;
	}
    int GetBuyEndTime() const {
        return BuyEndTime;
	}
};
