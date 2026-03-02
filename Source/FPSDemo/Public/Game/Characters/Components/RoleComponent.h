// RoleComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Game/Characters/CharacterRole.h"
#include "RoleComponent.generated.h"


// Native multicast delegate (same style as your SpikeComponent)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnRoleChanged, ECharacterRole /*OldRole*/, ECharacterRole /*NewRole*/);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FPSDEMO_API URoleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URoleComponent();

	ECharacterRole GetRole() const { return CurrentRole; }
	bool SetRoleAuthoritative(ECharacterRole NewRole);
	bool IsHuman() const { return CurrentRole == ECharacterRole::Human; }
	bool IsZombie() const { return CurrentRole == ECharacterRole::Zombie; }

	// Fired on both server and clients when role actually changes.
	FOnRoleChanged OnRoleChanged;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// Replicated role state
	UPROPERTY(ReplicatedUsing = OnRep_CurrentRole)
	ECharacterRole CurrentRole = ECharacterRole::Human;

	bool SetRole_Internal(ECharacterRole NewRole);

	UFUNCTION()
	void OnRep_CurrentRole(ECharacterRole OldRole);

	void HandleRoleChanged(ECharacterRole OldRole, ECharacterRole NewRole);
};
