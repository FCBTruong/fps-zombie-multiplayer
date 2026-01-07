#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISenseConfig_Damage.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Game/ShooterGameState.h"
#include "Bot/BotRole.h"
#include "Characters/CharacterRole.h"
#include "BotAIController.generated.h"

class ABaseCharacter;

namespace BotBBKeys
{
    inline const FName TargetActor(TEXT("Obj_TargetActor"));
    inline const FName HasLOS(TEXT("B_HasLineSight"));
    inline const FName TargetLocation(TEXT("Vec_TargetLocation"));
    inline const FName MatchMode(TEXT("E_MatchMode"));
	inline const FName SpikeRole(TEXT("E_Role"));
	inline const FName SpikeActor(TEXT("Obj_SpikeActor"));
	inline const FName IsAttacker(TEXT("B_IsAttacker"));
	inline const FName PlantLocation(TEXT("Vec_PlantLocation"));
	inline const FName CharacterRole(TEXT("E_CharacterRole"));
	inline const FName ScoutLocation(TEXT("Vec_ScoutLocation"));
	inline const FName HasLineSight(TEXT("B_HasLineSight"));
	inline const FName HoldLocation(TEXT("Vec_HoldLocation"));
}

UCLASS()
class FPSDEMO_API ABotAIController : public AAIController
{
    GENERATED_BODY()

public:
    ABotAIController();

    void ResetAIState();
    void StartPlantingSpike();
    void StartDefusingSpike();
    void RequestFireOnce();

    void SetTargetActor(ABaseCharacter* NewTarget);
	void SetMatchMode(EMatchMode NewMode);
	void SetSpikeRole(EBotRole NewRole);
	void SetSpikeActor(AActor* NewSpikeActor);
    void SetIsAttacker(bool bAttacker);
	void SetPlantLocation(const FVector& NewLocation);
	void SetCharacterRole(ECharacterRole NewRole);
	void SetScoutLocation(const FVector& NewLocation);
	void SetHasLineSight(bool bLineSight);
	void SetHoldLocation(const FVector& NewLocation);

	EBotRole GetSpikeRole() const { return SpikeRole; }
	ABaseCharacter* GetTargetActor() const;
	bool GetIsAttacker() const { return bIsAttacker; }
	bool HasLineOfSight() const { return bHasLineSight; }
    ABaseCharacter* GetBotChar() const { return CachedChar.Get(); }
protected:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnUnPossess() override;
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
    UFUNCTION()
    void OnTargetPerceptionUpdated(
        AActor* Actor,
        FAIStimulus Stimulus);
    void OnAmmoChanged(int32 Clip, int32 Reserve);
private:
    UPROPERTY(VisibleAnywhere)
    class UAIPerceptionComponent* PerceptionComp;

    UPROPERTY(VisibleAnywhere)
    class UAISenseConfig_Sight* SightConfig;

    UPROPERTY()
    UAISenseConfig_Damage* DamageConfig;

    ABaseCharacter* TargetActor;
    EMatchMode CurrentMatchMode;
    EBotRole SpikeRole;
    AActor* SpikeActor;
    bool bIsAttacker;
    FVector PlantLocation;
    ECharacterRole CharacterRole;
	FVector ScoutLocation;
	FVector HoldLocation;
	bool bHasLineSight;

    TWeakObjectPtr<ABaseCharacter> CachedChar;

    void BindPawn(APawn* InPawn);
    void UnbindPawn();
};
