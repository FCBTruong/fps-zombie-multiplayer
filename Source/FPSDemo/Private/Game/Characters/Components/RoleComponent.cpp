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

	// Initialize only on server (authoritative).
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		// Set initial role once. This will replicate to clients.
		SetRole_Internal(InitialRole);
	}
}

void URoleComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(URoleComponent, CurrentRole);
}

void URoleComponent::RequestSetRole(ECharacterRole NewRole)
{
	if (!GetOwner())
		return;

	// If already server, set directly.
	if (GetOwner()->HasAuthority())
	{
		SetRoleAuthoritative(NewRole);
		return;
	}

	// Client -> server request
	ServerSetRole(NewRole);
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
	OnRoleChanged.Broadcast(OldRole, NewRole);

	return true;
}

void URoleComponent::OnRep_CurrentRole(ECharacterRole OldRole)
{
	// This runs on clients when CurrentRole changes via replication.
	OnRoleChanged.Broadcast(OldRole, CurrentRole);
}

void URoleComponent::ServerSetRole_Implementation(ECharacterRole NewRole)
{
	// Server decides if allowed. Keep logic here or delegate to GameMode/authority systems.
	SetRole_Internal(NewRole);
}
