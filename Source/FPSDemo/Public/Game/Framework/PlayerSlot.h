// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Game/Types/TeamId.h"
#include "PlayerSlot.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnReplicatedPawnChanged)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateConnectedStatus, bool /*bIsConnected*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateTeamId, ETeamId);

UCLASS()
class FPSDEMO_API APlayerSlot : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlayerSlot();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetBackendUserId(int InBackendUserId) { BackendUserId = InBackendUserId; }
	void SetPlayerName(const FString& InPlayerName) { PlayerName = InPlayerName; }
	void SetTeamId(ETeamId InTeamId);
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
	void SetPawn(APawn* InPawn);
	bool IsConnected() const { return bIsConnected; }
	void SetIsConnected(bool bInIsConnected);
	int GetCharacterSkin() const { return CharacterSkin; }
	void SetCharacterSkin(int InCharacterSkin) { CharacterSkin = InCharacterSkin; }
	const FString& GetPlayerName() const{ return PlayerName; }
	const FString& GetAvatar() const { return Avatar; }
	void SetAvatar(const FString& InAvatar) { Avatar = InAvatar; }
	void SetCrosshairCode(const FString& InCrosshairCode) { CrosshairCode = InCrosshairCode; }
	const FString& GetCrosshairCode() const { return CrosshairCode; }
	void SetController(AController* InController) { Controller = InController; }
	AController* GetController() const;

	UFUNCTION()
	void OnRep_Pawn();

	UFUNCTION()
	void OnRep_TeamId();

	UFUNCTION()
	void OnRep_IsConnected();

	FOnReplicatedPawnChanged OnReplicatedPawnChanged;
	FOnUpdateConnectedStatus OnUpdateConnectedStatus;
	FOnUpdateTeamId OnUpdateTeamId;
private:
	UPROPERTY(Replicated)
	int BackendUserId;

	UPROPERTY(ReplicatedUsing = OnRep_TeamId)
	ETeamId TeamId;

	UPROPERTY(Replicated)
	FString PlayerName = "";

	UPROPERTY(Replicated)
	FString Avatar = "";

	FString CrosshairCode = "";

	UPROPERTY(Replicated)
	bool bIsBot;

	UPROPERTY(ReplicatedUsing=OnRep_IsConnected)
	bool bIsConnected;
	
	UPROPERTY(Replicated)
	int Kills;

	UPROPERTY(Replicated)
	int Deaths;

	UPROPERTY(Replicated)
	int Assists;

	UPROPERTY(ReplicatedUsing=OnRep_Pawn)
	TObjectPtr<APawn> Pawn = nullptr;

	int32 CharacterSkin;
	TWeakObjectPtr<AController> Controller;
};
