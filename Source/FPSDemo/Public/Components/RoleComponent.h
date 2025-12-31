// RoleComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RoleComponent.generated.h"

// Keep this enum generic. It is "form/archetype" of the character.
// Zombie mode uses more values; bomb mode can stay at Human only.
UENUM(BlueprintType)
enum class ECharacterRole : uint8
{
	Human	UMETA(DisplayName = "Human"),
	Zombie	UMETA(DisplayName = "Zombie"),
	Hero	UMETA(DisplayName = "Hero")
};

// Native multicast delegate (same style as your SpikeComponent)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnRoleChanged, ECharacterRole /*OldRole*/, ECharacterRole /*NewRole*/);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FPSDEMO_API URoleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URoleComponent();

	virtual void BeginPlay() override;

	// Client-side request (will call ServerSetRole).
	// In many games you only allow server/game mode to call this.
	void RequestSetRole(ECharacterRole NewRole);

	// Server-side immediate set (no RPC). Use from GameMode/GameState/server authority code.
	bool SetRoleAuthoritative(ECharacterRole NewRole);

	ECharacterRole GetRole() const { return CurrentRole; }

	bool IsHuman() const { return CurrentRole == ECharacterRole::Human; }
	bool IsZombie() const { return CurrentRole == ECharacterRole::Zombie; }

	// Fired on both server and clients when role actually changes.
	FOnRoleChanged OnRoleChanged;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// Replicated role state
	UPROPERTY(ReplicatedUsing = OnRep_CurrentRole)
	ECharacterRole CurrentRole = ECharacterRole::Human;

	// Optional: default role for BeginPlay on server (if you want to override per pawn BP)
	UPROPERTY(EditDefaultsOnly, Category = "Role")
	ECharacterRole InitialRole = ECharacterRole::Human;

	// Optional: prevent redundant changes
	bool SetRole_Internal(ECharacterRole NewRole);

	UFUNCTION()
	void OnRep_CurrentRole(ECharacterRole OldRole);

	// RPC: clients request, server decides
	UFUNCTION(Server, Reliable)
	void ServerSetRole(ECharacterRole NewRole);
};
