// WeaponActionStateComponent.cpp

#include "Components/ActionStateComponent.h"
#include "Net/UnrealNetwork.h"

UActionStateComponent::UActionStateComponent()
{
    SetIsReplicatedByDefault(true);
}

void UActionStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UActionStateComponent, State);
}

bool UActionStateComponent::CanTransition(EActionState From, EActionState To) const
{
    // Keep the rules centralized here.
    // Adjust based on your existing enum values.

    // Example rules from your current code:
    if (From == EActionState::Reloading && To == EActionState::Firing)
        return false;

    if (From == EActionState::Planting && To != EActionState::Idle)
        return false;

    if (From == EActionState::Defusing && To != EActionState::Idle)
        return false;

    // Common-sense blocks (optional but recommended):
    // If already doing an action, disallow starting another unless going back to Idle.
    if (From != EActionState::Idle && To != EActionState::Idle)
        return false;

    // Allow Idle -> anything
    // Allow anything -> Idle
    return true;
}

bool UActionStateComponent::TrySetState(EActionState NewState)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return false;

    if (State == NewState)
        return true;

    if (!CanTransition(State, NewState))
        return false;

    const EActionState Old = State;
    State = NewState;
    HandleStateChanged(Old, State);
    return true;
}

void UActionStateComponent::ForceSetState(EActionState NewState)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return;

    if (State == NewState)
        return;

    const EActionState Old = State;
    State = NewState;
    HandleStateChanged(Old, State);
}

void UActionStateComponent::OnRep_State(EActionState OldState)
{
    HandleStateChanged(OldState, State);
}

void UActionStateComponent::HandleStateChanged(EActionState OldState, EActionState NewState)
{
    OnStateChanged.Broadcast(OldState, NewState);

}
