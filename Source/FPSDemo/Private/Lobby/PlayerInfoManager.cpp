// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/PlayerInfoManager.h"
#include "Kismet/GameplayStatics.h"
#include "Data/MySaveGame.h"
#include "Utils/GameUtils.h"
#include "Network/NetworkManager.h"

UPlayerInfoManager::UPlayerInfoManager()
{
}

void UPlayerInfoManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadOrCreateLocalInfo();

    Collection.InitializeDependency<UNetworkManager>();

    UNetworkManager* Network =
        Collection.InitializeDependency<UNetworkManager>();

    check(Network);

    Network->RegisterListener(this);
	UE_LOG(LogTemp, Log, TEXT("OK PlayerInfoManager initialized"));
}

void UPlayerInfoManager::SetGuestId(const FString& InGuestId)
{
	GuestId = InGuestId;
}

void UPlayerInfoManager::SetPlayerName(const FString& InPlayerName)
{
	PlayerName = InPlayerName;
}

FString UPlayerInfoManager::GetPlayerName() const
{
	return PlayerName;
}

void UPlayerInfoManager::SetUserId(int InUserId)
{
	UserId = InUserId;
}

int UPlayerInfoManager::GetUserId() const
{
	return UserId;
}

void UPlayerInfoManager::SetAvatar(const FString& InAvatar)
{
	Avatar = InAvatar;
}

FString UPlayerInfoManager::GetAvatar() const
{
	return Avatar;
}

FString UPlayerInfoManager::GetGuestId() const
{
	return GuestId;
}

UPlayerInfoManager* UPlayerInfoManager::Get(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}
	return GameInstance->GetSubsystem<UPlayerInfoManager>();
}

void UPlayerInfoManager::LoadOrCreateLocalInfo()
{
    using namespace PlayerLocalInfoConst;

    UMySaveGame* Save = nullptr;

    if (UGameplayStatics::DoesSaveGameExist(SLOT, USER_INDEX))
    {
        Save = Cast<UMySaveGame>(
            UGameplayStatics::LoadGameFromSlot(SLOT, USER_INDEX)
        );
    }

    if (!Save)
    {
        Save = Cast<UMySaveGame>(
            UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass())
        );
    }

    if (!Save)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create SaveGame object"));
        return;
    }

    if (Save->GuestId.IsEmpty())
    {
        Save->GuestId = GameUtils::GenerateMd5Token();
    }

    // for testing editor
    if (GIsEditor)
    {
        // Running in Editor (PIE or Standalone launched from editor)
        Save->GuestId = GameUtils::GenerateMd5Token();
    }
    else
    {
        // Running in packaged game
    }

    if (Save->PlayerName.IsEmpty())
    {
        Save->PlayerName = TEXT("Player");
    }

    GuestId = Save->GuestId;
    PlayerName = Save->PlayerName;

    UGameplayStatics::SaveGameToSlot(Save, SLOT, USER_INDEX);
}

void UPlayerInfoManager::OnPacketReceived(
    ECmdId CmdId,
    const std::string& Payload
)
{
    // Handle incoming packets if needed
    switch (CmdId)
    {
    case ECmdId::LOGIN_REPLY:
    {
        game::net::LoginReply Reply;
        if (!Reply.ParseFromString(Payload))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to parse LoginReply"));
            return;
        }
        SetUserId(Reply.user_id());
        break;
    }
    default:
        break;
    }
}