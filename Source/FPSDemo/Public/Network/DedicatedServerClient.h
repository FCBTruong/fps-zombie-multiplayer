// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Lobby/RoomData.h"
#include "Http.h"
/**
 * 
 */
class FPSDEMO_API DedicatedServerClient
{
public:
    DedicatedServerClient();
    ~DedicatedServerClient();
    void GetMatchInfo(TFunction<void(bool bOk, const FString& ResponseBody)> Callback);
    void NotifyReady(TFunction<void(bool bOk, const FString& ResponseBody)> Callback);
    void NotifyFinish(const FString& ResultJson,
        TFunction<void(bool bOk, const FString& ResponseBody)> Callback);

    void SetBearerToken(const FString& InBearerToken);
    static bool ParseMatchInfo(const FString& JsonString, FRoomData& OutMatchInfo);
private:
    FString BaseUrl;
    FString BearerToken;

    void SendJsonRequest(
        const FString& Path,
        const FString& Verb,
        const FString& JsonBody,
        TFunction<void(bool bOk, const FString& ResponseBody)> Callback);

    static void ApplyCommonHeaders(const TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& Req, const FString& Bearer);
};