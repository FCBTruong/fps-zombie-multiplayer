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

UCLASS()
class FPSDEMO_API AShooterGameState : public AGameState
{
    GENERATED_BODY()

protected:
	EMyMatchState CurrentMatchState = EMyMatchState::WAITING_TO_START;
public:
	AShooterGameState();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    TMap<int32, FPickupData> ItemsOnMap;
	TArray<FPickupData> GetItemsOnMap() const;
    
    UFUNCTION(NetMulticast, UnReliable)
    void MulticastKillNotify(AMyPlayerState* Killer, AMyPlayerState* Victim, UWeaponData* DamageCauser, bool bWasHeadShot);
    void SetMatchState(EMyMatchState NewState) {
        CurrentMatchState = NewState;
    }
    EMyMatchState GetMatchState() const {
		return CurrentMatchState;
	}
};
