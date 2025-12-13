#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISenseConfig_Damage.h"
#include "BotAIController.generated.h"

UCLASS()
class FPSDEMO_API ABotAIController : public AAIController
{
    GENERATED_BODY()

public:
    ABotAIController();

    void ResetAIState();
    void StartPlantingSpike();
    void StartDefusingSpike();
protected:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
    UFUNCTION()
    void OnTargetPerceptionUpdated(
        AActor* Actor,
        FAIStimulus Stimulus);
private:
    UPROPERTY(VisibleAnywhere)
    class UAIPerceptionComponent* PerceptionComp;

    UPROPERTY(VisibleAnywhere)
    class UAISenseConfig_Sight* SightConfig;

    UPROPERTY()
    UAISenseConfig_Damage* DamageConfig;

    AActor* CurrentTarget;
};
