#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "PickupData.h"
#include "ShooterGameState.generated.h"

UCLASS()
class FPSDEMO_API AShooterGameState : public AGameState
{
    GENERATED_BODY()
public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    UPROPERTY()
    TMap<int32, FPickupData> ItemsOnMap;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_UpdateItemsOnMap(const TArray<FPickupData>& NewItems);
};
