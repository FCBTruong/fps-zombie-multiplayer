// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/TeamId.h"
#include "PlayerSlot.generated.h"

UCLASS()
class FPSDEMO_API APlayerSlot : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlayerSlot();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetBackendUserId(int InBackendUserId) { BackendUserId = InBackendUserId; }
	void SetTeamId(ETeamId InTeamId) { TeamId = InTeamId; }
	void SetIsBot(bool bInIsBot) { bIsBot = bInIsBot; }
	void SetKills(int InKills) { Kills = InKills; }
	void SetDeaths(int InDeaths) { Deaths = InDeaths; }
	void SetAssists(int InAssists) { Assists = InAssists; }
	int GetKills() const { return Kills; }
	int GetDeaths() const { return Deaths; }
	int GetAssists() const { return Assists; }
	void AddKill() { ++Kills; }
	void AddDeath() { ++Deaths; }
	void AddAssist() { ++Assists; }
	int GetBackendUserId() const { return BackendUserId; }
	ETeamId GetTeamId() const { return TeamId; }
	bool IsBot() const { return bIsBot; }
	APawn* GetPawn() const { return Pawn; }
	void SetPawn(APawn* InPawn) { Pawn = InPawn; }
	bool IsConnected() const { return bIsConnected; }
	void SetIsConnected(bool bInIsConnected) { bIsConnected = bInIsConnected; }
	int GetCharacterSkin() const { return CharacterSkin; }
	void SetCharacterSkin(int InCharacterSkin) { CharacterSkin = InCharacterSkin; }
private:
	UPROPERTY(Replicated)
	int BackendUserId;

	UPROPERTY(Replicated)
	ETeamId TeamId;

	UPROPERTY(Replicated)
	bool bIsBot;

	UPROPERTY(Replicated)
	bool bIsConnected;
	
	UPROPERTY(Replicated)
	int Kills;

	UPROPERTY(Replicated)
	int Deaths;

	UPROPERTY(Replicated)
	int Assists;

	UPROPERTY(Replicated)
	TObjectPtr<APawn> Pawn = nullptr;

	int32 CharacterSkin;
};
