// NetworkManager.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IWebSocket.h"
#include "NetworkManager.generated.h"


UCLASS()
class FPSDEMO_API UNetworkManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void Connect();

private:
    // UE 5.6 OnRawMessage signature is (const void*, SIZE_T, SIZE_T)
    void OnRawMessage(const void* Data, SIZE_T Size, SIZE_T BytesRemaining);

    void HandleConnected();
    void HandleConnectionError(const FString& Error);
    void HandleClosed(int32 StatusCode, const FString& Reason, bool bWasClean);

private:
    TSharedPtr<IWebSocket> Socket;
};
