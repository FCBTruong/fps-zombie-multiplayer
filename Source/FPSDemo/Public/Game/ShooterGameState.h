#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Pickup/PickupData.h"
#include "Controllers/MyPlayerState.h"
#include "Weapons/WeaponData.h"
#include "ShooterGameState.generated.h"

enum EMyMatchState {
	WAITING_TO_START,
    PLAYING,
    ROUND_ENDED,
	GAME_ENDED
};


DECLARE_MULTICAST_DELEGATE(FOnUpdateScore)

UCLASS()
class FPSDEMO_API AShooterGameState : public AGameState
{
    GENERATED_BODY()

protected:
	EMyMatchState CurrentMatchState = EMyMatchState::WAITING_TO_START;

	UPROPERTY(Replicated)
    FName AttackerTeam = FName(TEXT("A"));

    UPROPERTY(ReplicatedUsing = OnRep_Score)
    int TeamAScore = 0;

    UPROPERTY(ReplicatedUsing = OnRep_Score)
    int TeamBScore = 0;
    
    UFUNCTION()
    void OnRep_Score();

public:
	AShooterGameState();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    UFUNCTION(NetMulticast, UnReliable)
    void MulticastKillNotify(AMyPlayerState* Killer, AMyPlayerState* Victim, UWeaponData* DamageCauser, bool bWasHeadShot);
    void SetMatchState(EMyMatchState NewState) {
        CurrentMatchState = NewState;
    }
    EMyMatchState GetMatchState() const {
		return CurrentMatchState;
	}
	UFUNCTION(NetMulticast, UnReliable)
	void Multicast_RoundResult(FName WinningTeam);
    FName GetAttackerTeam() const {
        return AttackerTeam;
	}
    FName GetDefenderTeam() const {
        return AttackerTeam == FName(TEXT("A")) ? FName(TEXT("B")) : FName(TEXT("A"));
	}

    FOnUpdateScore OnUpdateScore;

    void AddScoreTeam(FName TeamId, int ScoreToAdd);
    int GetScoreTeam(FName TeamId) const;
    void SetAttackerTeam(FName NewAttackerTeam) {
        AttackerTeam = NewAttackerTeam;
    }
};
