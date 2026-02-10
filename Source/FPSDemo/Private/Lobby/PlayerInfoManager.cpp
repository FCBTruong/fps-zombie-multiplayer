// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/PlayerInfoManager.h"
#include "Kismet/GameplayStatics.h"
#include "Data/MySaveGame.h"
#include "Utils/GameUtils.h"
#include "Network/NetworkManager.h"
#include "GameConstants.h"

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

const FString& UPlayerInfoManager::GetPlayerName() const
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

const FString& UPlayerInfoManager::GetAvatar() const
{
	return Avatar;
}

const FString& UPlayerInfoManager::GetGuestId() const
{
    if (!CheatGuestId.IsEmpty()) {
        return CheatGuestId;
	}
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

    if (Save->PlayerName.IsEmpty())
    {
        Save->PlayerName = TEXT("Player");
    }

    if (Save->Avatar.IsEmpty())
    if (Save->Avatar.IsEmpty())
    {
        int32 AvatarId = FGameConstants::AVATAR_IDS[
            FMath::RandRange(0, UE_ARRAY_COUNT(FGameConstants::AVATAR_IDS) - 1)
        ];

        Save->Avatar = FString::FromInt(AvatarId);
	}

    GuestId = Save->GuestId;
    PlayerName = Save->PlayerName;
	Avatar = Save->Avatar;
	CrosshairCode = Save->CrosshairCode;

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

void UPlayerInfoManager::Login(int id)
{
	CheatGuestId = FString::FromInt(id);

	UE_LOG(LogTemp, Log, TEXT("UPlayerInfoManager::Login with CheatGuestId: %s"), *CheatGuestId);
    // reopen 
    UGameplayStatics::OpenLevel(this, FGameConstants::LEVEL_LOBBY);
}

void UPlayerInfoManager::SetCrosshairCode(const FString& InCrosshairCode)
{
    CrosshairCode = InCrosshairCode;

	// Save to local storage
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
    Save->CrosshairCode = CrosshairCode;
	UGameplayStatics::SaveGameToSlot(Save, SLOT, USER_INDEX);
}

const FString& UPlayerInfoManager::GetCrosshairCode() const
{
    return CrosshairCode;
}