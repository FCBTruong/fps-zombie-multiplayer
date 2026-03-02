// Fill out your copyright notice in the Description page of Project Settings.


#include "Modules/Lobby/LoginUI.h"
#include "Network/NetworkManager.h"
#include "Game/GameManager.h"
#include "Shared/System/PlayerInfoManager.h"

void ULoginUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (LoginBtn) {
		LoginBtn->OnClicked.AddDynamic(this, &ULoginUI::TryLogin);
	}

	TryLogin();
}

void ULoginUI::TryLogin()
{
	auto GameInstance = GetWorld()->GetGameInstance();
	UNetworkManager* NetworkManager =
		GameInstance->GetSubsystem<UNetworkManager>();

	if (NetworkManager == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoginUI: NetworkManager subsystem not found"));
		return;
	}
	// send packet login request
	UPlayerInfoManager* PlayerInfoMgr = UPlayerInfoManager::Get(GetWorld());
	if (PlayerInfoMgr == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoginUI: PlayerInfoMgr subsystem not found"));
		return;
	}

	game::net::LoginRequest Login;
	Login.set_guest_id(TCHAR_TO_UTF8(*PlayerInfoMgr->GetGuestId()));
	Login.set_player_name(TCHAR_TO_UTF8(*PlayerInfoMgr->GetPlayerName()));
	Login.set_avatar(TCHAR_TO_UTF8(*PlayerInfoMgr->GetAvatar()));
	Login.set_crosshair_code(TCHAR_TO_UTF8(*PlayerInfoMgr->GetCrosshairCode()));

	NetworkManager->SendPacket(ECmdId::LOGIN, Login);
}