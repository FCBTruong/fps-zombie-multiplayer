#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "BotAIController.generated.h"

UCLASS()
class FPSDEMO_API ABotAIController : public AAIController
{
    GENERATED_BODY()

public:
    ABotAIController();

    void ResetAIState();
    void StartPlantingSpike();
protected:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
private:
    UPROPERTY(VisibleAnywhere)
    class UAIPerceptionComponent* PerceptionComp;

    UPROPERTY(VisibleAnywhere)
    class UAISenseConfig_Sight* SightConfig;

    AActor* CurrentTarget;
};
