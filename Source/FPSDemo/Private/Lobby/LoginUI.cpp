// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LoginUI.h"
#include "Network/NetworkManager.h"
#include "Game/GameManager.h"

void ULoginUI::NativeConstruct()
{
	Super::NativeConstruct();

	TryLogin();
}

void ULoginUI::TryLogin()
{
	// send packet login request
	if (UNetworkManager* NetworkManager =
		GetWorld()->GetGameInstance()->GetSubsystem<UNetworkManager>())
	{
		game::net::LoginRequest Login;
		Login.set_player_name("PlayerOne");
		Login.set_avatar("1");
		UGameManager* GMR = UGameManager::Get(GetWorld());
		if (GMR == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("LoginUI: GameManager subsystem not found"));
			return;
		}
		Login.set_guest_id(TCHAR_TO_UTF8(*GMR->GuestId));

		UE_LOG(LogTemp, Warning, TEXT("UNetworkManager: debuggg subsystem not found"));
		NetworkManager->SendPacket(ECmdId::LOGIN, Login);
	}
}