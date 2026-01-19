// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "NetBoostrapSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UNetBoostrapSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    // Self-host (listen server, EOS P2P). Orchestrator decides members to join.
    void StartSelfHost(const FName& Map, const FString& InMatchId, const FString& InJoinKey);

    // Dedicated server (public internet). Orchestrator provides IP:Port for clients.
    void JoinDedicated(const FString& ServerIpPort); // e.g. "12.34.56.78:7777"
    void JoinSelfHostByMatch(const FString& InMatchId, const FString& InJoinKey);
private:
    IOnlineIdentityPtr Identity;
    IOnlineSessionPtr Session;
    FName MapName;
    TSharedPtr<FOnlineSessionSearch> Search;
    bool bIsHost = false;
    FString MatchId;
    FString JoinKey;
    FDelegateHandle PostLoadHandle;
    FDelegateHandle DestroyHandle;
    FDelegateHandle CreateHandle;
    FDelegateHandle FindHandle;
	FDelegateHandle LoginHandle;
	FDelegateHandle JoinHandle;

    // Self-host
    void LoginDeviceId();
    void OnLoginComplete(int32, bool bOk, const FUniqueNetId&, const FString&);
    void CreateMatchSession();
    void OnCreateSessionComplete(FName, bool bOk);
    void OpenListen();
    void OnJoinComplete(FName, EOnJoinSessionCompleteResult::Type);
    void OnFindSessionsComplete(bool bOk);
    void RegisterPostLoadMapDelegate();
    void OnHostMapLoaded(UWorld* LoadedWorld);
    void DoCreateSession();
    void OnDestroySessionComplete(FName SessionName, bool bSuccess);
    void DoFindSessions();
    bool InitOSS();
};
