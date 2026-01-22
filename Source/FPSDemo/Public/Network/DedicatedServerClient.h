// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
/**
 * 
 */
class FPSDEMO_API DedicatedServerClient
{
public:
    DedicatedServerClient();

    void GetMatchInfo(TFunction<void(bool bOk, const FString& ResponseBody)> Callback);
    void NotifyReady(const FString& ServerInstanceId, TFunction<void(bool bOk, const FString& ResponseBody)> Callback);
    void NotifyFinish(const FString& ServerInstanceId, const FString& EventId, const FString& ResultJson,
        TFunction<void(bool bOk, const FString& ResponseBody)> Callback);

    void SetBearerToken(const FString& InBearerToken);

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