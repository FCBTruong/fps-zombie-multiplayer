// NetworkManager.cpp

#include "Network/NetworkManager.h"
#include "WebSocketsModule.h"
#include "Async/Async.h"
#include "Network/MyNetworkSettings.h"
#include "Lobby/PlayerInfoManager.h"

void UNetworkManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Dispatcher = MakeUnique<FPacketDispatcher>(this);
    // log token
	UE_LOG(LogTemp, Log, TEXT("Player Token: %s"), *Token);
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

    FString Url;

#if UE_BUILD_SHIPPING
    Url = GetDefault<UMyNetworkSettings>()->ProdWebSocketUrl;
#else
    Url = GetDefault<UMyNetworkSettings>()->DevWebSocketUrl;
#endif

	UE_LOG(LogTemp, Log, TEXT("WebSocket URL: %s"), *Url);

    Socket = FWebSocketsModule::Get().CreateWebSocket(Url);

    Socket->OnConnected().AddUObject(this, &UNetworkManager::HandleConnected);
    Socket->OnConnectionError().AddUObject(this, &UNetworkManager::HandleConnectionError);
    Socket->OnClosed().AddUObject(this, &UNetworkManager::HandleClosed);

    // UE 5.6 expects 3 params here
    Socket->OnRawMessage().AddUObject(this, &UNetworkManager::OnRawMessage);

    Socket->Connect();

	UE_LOG(LogTemp, Log, TEXT("WebSocket connection initiated"));
}

void UNetworkManager::HandleConnected()
{
    UE_LOG(LogTemp, Log, TEXT("WebSocket connected"));
	OnNetworkConnected.Broadcast();
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

    AsyncTask(ENamedThreads::GameThread,
        [this, Packet = std::move(Packet)]()
        {
            Dispatcher->Dispatch(Packet);
        });
}

void UNetworkManager::SendPacket(ECmdId CmdId, const google::protobuf::Message& Msg)
{
    if (!Socket.IsValid() || !Socket->IsConnected())
    {
        UE_LOG(LogTemp, Warning, TEXT("UNetworkManager: WebSocket not connected"));
        return;
    }

    // Serialize inner protobuf message
    std::string Payload;
    if (!Msg.SerializeToString(&Payload))
    {
        UE_LOG(LogTemp, Error, TEXT("UNetworkManager: Failed to serialize payload"));
        return;
    }

    // Wrap into Packet
    game::net::Packet Packet;
    Packet.set_cmd_id(static_cast<int32>(CmdId));
	Packet.set_token(TCHAR_TO_UTF8(*Token));
    Packet.set_payload(Payload);

    // Serialize Packet
    std::string PacketBytes;
    if (!Packet.SerializeToString(&PacketBytes))
    {
        UE_LOG(LogTemp, Error, TEXT("UNetworkManager: Failed to serialize Packet"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("UNetworkManager: Sending Packet CmdId: %d, Size: %d bytes"),
        static_cast<int32>(CmdId),
        static_cast<int32>(PacketBytes.size())
	);

    // Send binary WebSocket frame
    Socket->Send(
        (void*)PacketBytes.data(),
        PacketBytes.size(),
        /*bIsBinary=*/true
    );
}

void UNetworkManager::HandleLoginSuccess(const game::net::LoginReply& Reply)
{
	Token = Reply.token().c_str();

	UPlayerInfoManager::Get(GetWorld())->SetUserId(Reply.user_id());

    UE_LOG(LogTemp, Log, TEXT("Login successful, token saved"));
    OnLoginSuccess.Broadcast();
}

void UNetworkManager::RegisterListener(IPacketListener* Listener)
{
    if (Dispatcher) {
        Dispatcher->RegisterListener(Listener);
    }
}