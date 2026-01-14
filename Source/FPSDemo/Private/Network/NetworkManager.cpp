// NetworkManager.cpp

#include "Network/NetworkManager.h"
#include "WebSocketsModule.h"
#include "Async/Async.h"
#include "game.pb.h"

void UNetworkManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Connect();
}

void UNetworkManager::Deinitialize()
{
    if (Socket.IsValid())
    {
        Socket->OnConnected().RemoveAll(this);
        Socket->OnConnectionError().RemoveAll(this);
        Socket->OnClosed().RemoveAll(this);
        Socket->OnRawMessage().RemoveAll(this);

        Socket->Close();
        Socket.Reset();
    }

    Super::Deinitialize();
}

void UNetworkManager::Connect()
{
    UE_LOG(LogTemp, Log, TEXT("Connecting to WebSocket server..."));

    Socket = FWebSocketsModule::Get().CreateWebSocket(TEXT("ws://localhost:5044/ws"));

    Socket->OnConnected().AddUObject(this, &UNetworkManager::HandleConnected);
    Socket->OnConnectionError().AddUObject(this, &UNetworkManager::HandleConnectionError);
    Socket->OnClosed().AddUObject(this, &UNetworkManager::HandleClosed);

    // UE 5.6 expects 3 params here
    Socket->OnRawMessage().AddUObject(this, &UNetworkManager::OnRawMessage);

    Socket->Connect();
}

void UNetworkManager::HandleConnected()
{
    UE_LOG(LogTemp, Log, TEXT("WebSocket connected"));
}

void UNetworkManager::HandleConnectionError(const FString& Error)
{
    UE_LOG(LogTemp, Error, TEXT("WebSocket connection error: %s"), *Error);
}

void UNetworkManager::HandleClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    UE_LOG(LogTemp, Warning, TEXT("WebSocket closed. Code=%d Clean=%d Reason=%s"),
        StatusCode, bWasClean ? 1 : 0, *Reason);
}

void UNetworkManager::OnRawMessage(const void* Data, SIZE_T Size, SIZE_T /*BytesRemaining*/)
{
    game::net::Packet Packet;
    if (!Packet.ParseFromArray(Data, static_cast<int>(Size)))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse Protobuf Envelope"));
        return;
    }

    const uint32 CmdId = Packet.cmd_id();

    AsyncTask(ENamedThreads::GameThread, [this, Envelope = std::move(Packet), CmdId]()
        {
            UE_LOG(LogTemp, Log, TEXT("Received Packet Seq: %u"), CmdId);
        });
}
