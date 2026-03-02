// RoleComponent.cpp

#include "Game/Characters/Components/RoleComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "Game/AI/BotAIController.h"

URoleComponent::URoleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void URoleComponent::BeginPlay()
{
	Super::BeginPlay();
	HandleRoleChanged(CurrentRole, CurrentRole);
}

void URoleComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(URoleComponent, CurrentRole);
}

bool URoleComponent::SetRoleAuthoritative(ECharacterRole NewRole)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return false;

	return SetRole_Internal(NewRole);
}

bool URoleComponent::SetRole_Internal(ECharacterRole NewRole)
{
	if (CurrentRole == NewRole)
		return false;

	const ECharacterRole OldRole = CurrentRole;
	CurrentRole = NewRole;

	// On server, broadcast immediately as well (clients will broadcast via OnRep).
	HandleRoleChanged(OldRole, CurrentRole);

	return true;
}

void URoleComponent::OnRep_CurrentRole(ECharacterRole OldRole)
{
	HandleRoleChanged(OldRole, CurrentRole);
}

// Use to update visuals, meshes, etc. when role changes. Called on both server and clients.
void URoleComponent::HandleRoleChanged(ECharacterRole OldRole, ECharacterRole NewRole)
{
	OnRoleChanged.Broadcast(OldRole, NewRole);
}