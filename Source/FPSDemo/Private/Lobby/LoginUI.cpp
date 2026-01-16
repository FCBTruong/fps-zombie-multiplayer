// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LoginUI.h"
#include "Network/NetworkManager.h"
#include "Game/GameManager.h"
#include "Lobby/PlayerInfoManager.h"

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
	// send packet login request
	if (UNetworkManager* NetworkManager =
		GetWorld()->GetGameInstance()->GetSubsystem<UNetworkManager>())
	{
		game::net::LoginRequest Login;
	
		UPlayerInfoManager* PlayerInfoMgr = UPlayerInfoManager::Get(GetWorld());
		if (PlayerInfoMgr == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("LoginUI: PlayerInfoMgr subsystem not found"));
			return;
		}
		Login.set_guest_id(TCHAR_TO_UTF8(*PlayerInfoMgr->GetGuestId()));
		Login.set_player_name(TCHAR_TO_UTF8(*PlayerInfoMgr->GetPlayerName()));
		Login.set_avatar(TCHAR_TO_UTF8(*PlayerInfoMgr->GetAvatar()));

		UE_LOG(LogTemp, Warning, TEXT("UNetworkManager: debuggg subsystem not found"));
		NetworkManager->SendPacket(ECmdId::LOGIN, Login);
	}
}