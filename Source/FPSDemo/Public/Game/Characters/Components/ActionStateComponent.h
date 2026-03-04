// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActionStateComponent.generated.h"

class ABaseCharacter;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnActionStateChanged, EActionState /*Old*/, EActionState /*New*/);

UENUM(BlueprintType)
enum class EActionState : uint8
{
    Idle,

    Equipping,
    Unequipping,

    Firing,
    Reloading,
    Aiming,

    Throwing,
    Melee,

    Planting,
    Defusing,

	Disabled // can use when dead, stunned, etc.
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UActionStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UActionStateComponent();
    void Init();

    EActionState GetState() const { return State; }
    void ForceSetState(EActionState NewState); // use sparingly (reset on death)

    bool IsIdle() const { return State == EActionState::Idle; }
    bool CanTransition(EActionState From, EActionState To) const;
    // Server-only mutations
    bool TrySetState(EActionState NewState);
 
    // Useful gates
    bool CanEquipNow() const { return IsIdle(); }
    bool CanReloadNow() const;
    bool CanFireNow() const;
    bool CanThrowNow() const { return CanTransition(State, EActionState::Throwing); }
	bool CanMeleeNow() const { return CanTransition(State, EActionState::Melee); }
	bool CanPlantNow() const { return CanTransition(State, EActionState::Planting); }
	bool CanDefuseNow() const { return CanTransition(State, EActionState::Defusing); }
	bool IsInState(EActionState QueryState) const { return State == QueryState; }

    // delegates
    FOnActionStateChanged OnStateChanged;
protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
private:
	ABaseCharacter* OwnerCharacter = nullptr;

    UPROPERTY(ReplicatedUsing = OnRep_State)
    EActionState State = EActionState::Idle;

    UFUNCTION()
    void OnRep_State(EActionState OldState);

    void HandleStateChanged(EActionState OldState, EActionState NewState);
};
