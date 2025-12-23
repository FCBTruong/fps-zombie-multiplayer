// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActionStateComponent.generated.h"

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

    Disabled
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UActionStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UActionStateComponent();

    EActionState GetState() const { return State; }
    bool IsIdle() const { return State == EActionState::Idle; }

    // Queries
    bool CanTransition(EActionState From, EActionState To) const;

    // Server-only mutations
    bool TrySetState(EActionState NewState);
    void ForceSetState(EActionState NewState); // use sparingly (reset on death)

    // Useful gates
    bool CanEquipNow() const { return IsIdle(); }
    bool CanReloadNow() const;
    bool CanFireNow() const { return CanTransition(State, EActionState::Firing); }
    bool CanThrowNow() const { return CanTransition(State, EActionState::Throwing); }
protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
    FOnActionStateChanged OnStateChanged;

private:
    UPROPERTY(ReplicatedUsing = OnRep_State)
    EActionState State = EActionState::Idle;

    UFUNCTION()
    void OnRep_State(EActionState OldState);

    void HandleStateChanged(EActionState OldState, EActionState NewState);
};
