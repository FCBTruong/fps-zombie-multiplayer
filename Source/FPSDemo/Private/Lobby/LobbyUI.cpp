// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyUI.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Network/NetworkManager.h"

void ULobbyUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (UNetworkManager* NetworkManager =
		GetWorld()->GetGameInstance()->GetSubsystem<UNetworkManager>())
	{
		CachedNetworkManager = NetworkManager;

		NetworkManager->OnCreateRoom.AddUObject(
			this, &ULobbyUI::HandleCreateRoomSuccess);
	}

	CurrentRoomData.bIsActive = false;
	CurrentRoomData.bHasStarted = false;
	CurrentRoomData.Mode = EMatchMode::Spike;
	CurrentRoomData.bIsSelfHost = true;
	CurrentRoomData.Players.Add(PlayerRoomInfo{ TEXT("Player1"), 1 });
	CurrentRoomData.Players.Add(PlayerRoomInfo{ TEXT("Player1"), -1 });
	CurrentRoomData.Players.Add(PlayerRoomInfo{ TEXT("Player1"), -1 });
	CurrentRoomData.Players.Add(PlayerRoomInfo{ TEXT("Player1"), -1 });
	CurrentRoomData.Players.Add(PlayerRoomInfo{ TEXT("Player1"), -1 });
	CurrentRoomData.Players.Add(PlayerRoomInfo{ TEXT("Player1"), -1 });
	CurrentRoomData.Players.Add(PlayerRoomInfo{ TEXT("Player1"), -1 });
	CurrentRoomData.Players.Add(PlayerRoomInfo{ TEXT("Player1"), -1 });
	CurrentRoomData.Players.Add(PlayerRoomInfo{ TEXT("Player1"), -1 });
	CurrentRoomData.Players.Add(PlayerRoomInfo{ TEXT("Player1"), -1 });

	if (ZombieModeBtn)
	{
		ZombieModeBtn->OnClicked.AddDynamic(this, &ULobbyUI::OnZombieModeClicked);
	}
	if (BombModeBtn)
	{
		BombModeBtn->OnClicked.AddDynamic(this, &ULobbyUI::OnBombModeClicked);
	}
	if (BtnSelfHost)
	{
		BtnSelfHost->OnClicked.AddDynamic(this, &ULobbyUI::OnSelfHostClicked);
	}
	if (BtnDedicatedServer)
	{
		BtnDedicatedServer->OnClicked.AddDynamic(this, &ULobbyUI::OnDedicatedServerClicked);
	}
	if (StartBtn) {
		StartBtn->OnClicked.AddDynamic(this, &ULobbyUI::OnStartGameClicked);
	}
	if (AddBotTeam1Btn)
	{
		AddBotTeam1Btn->OnClicked.AddDynamic(this, &ULobbyUI::OnAddBotTeam1);
	}
	if (AddBotTeam2Btn)
	{
		AddBotTeam2Btn->OnClicked.AddDynamic(this, &ULobbyUI::OnAddBotTeam2);
	}
	if (CreateRoomBtn) {
		CreateRoomBtn->OnClicked.AddDynamic(this, &ULobbyUI::OnCreateRoomClicked);
	}



	// get all objects of type URoomPlayerSlotUI
	PlayerSlotUIs.Empty();

	TArray<UWidget*> AllWidgets;
	WidgetTree->GetAllWidgets(AllWidgets);

	for (UWidget* Widget : AllWidgets)
	{
		if (!Widget) continue;

		const FString& Name = Widget->GetName();

		if (Name.Contains(TEXT("WBP_RoomPlayerSlot")))
		{
			if (URoomPlayerSlotUI* SlotWidget = Cast<URoomPlayerSlotUI>(Widget))
			{
				PlayerSlotUIs.Add(SlotWidget);
				SlotWidget->OnDeletePlayer.AddUObject(this, &ULobbyUI::OnDeletePlayer);
			}
		}
	}

	if (ListPn) {
		ListPn->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (CreatePn) {
		CreatePn->SetVisibility(ESlateVisibility::Visible);
	}
}

void ULobbyUI::OnCreateRoomClicked() {
	if (UNetworkManager* NetworkManager =
		GetWorld()->GetGameInstance()->GetSubsystem<UNetworkManager>())
	{
		game::net::Empty Empty;
		NetworkManager->SendPacket(ECmdId::CREATE_ROOM, Empty);
	}
}

void ULobbyUI::UpdateRoomData() {
	for (int32 i = 0; i < PlayerSlotUIs.Num(); ++i)
	{
		const PlayerRoomInfo Info =
			CurrentRoomData.Players.IsValidIndex(i)
			? CurrentRoomData.Players[i]
			: PlayerRoomInfo();

		PlayerSlotUIs[i]->SetPlayerInfo(Info);
	}
	SetMatchMode(CurrentRoomData.Mode);
	SetHostMode(CurrentRoomData.bIsSelfHost);
}

void ULobbyUI::OnZombieModeClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Zombie Mode Clicked"));
	SetMatchMode(EMatchMode::Zombie);
}

void ULobbyUI::OnBombModeClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Bomb Mode Clicked"));
	SetMatchMode(EMatchMode::Spike);
}

void ULobbyUI::SetMatchMode(EMatchMode InMode)
{
	CurrentRoomData.Mode = InMode;

	if (InMode == EMatchMode::Spike)
	{
		VsTxt->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		VsTxt->SetVisibility(ESlateVisibility::Hidden);
	}

	SetButtonNormalColor(
		ZombieModeBtn,
		InMode == EMatchMode::Zombie ? LobbyUIColor::Selected : LobbyUIColor::Unselected);

	BombModeTickIcon->SetVisibility(
		InMode == EMatchMode::Spike ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	SetButtonNormalColor(
		BombModeBtn,
		InMode == EMatchMode::Spike ? LobbyUIColor::Selected : LobbyUIColor::Unselected);
	ZombieTickIcon->SetVisibility(
		InMode == EMatchMode::Zombie ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void ULobbyUI::OnSelfHostClicked()
{
	// start self-hosted server
	SetHostMode(true);
}

void ULobbyUI::OnDedicatedServerClicked()
{
	// start dedicated server
	SetHostMode(false);
}

void ULobbyUI::SetButtonNormalColor(UButton* Button, const FLinearColor& Color)
{
	if (!Button)
		return;

	FButtonStyle Style = Button->GetStyle();

	// Change ONLY normal state tint
	Style.Normal.TintColor = Color;
	Style.Pressed.TintColor = Color * 0.8f;
	Style.Hovered.TintColor = Color * 1.2f;

	Button->SetStyle(Style);
}

void ULobbyUI::SetHostMode(bool bIsSelfHost)
{
	CurrentRoomData.bIsSelfHost = bIsSelfHost;
	SetButtonNormalColor(
		BtnSelfHost,
		bIsSelfHost ? LobbyUIColor::Selected : LobbyUIColor::Unselected);
	SetButtonNormalColor(
		BtnDedicatedServer,
		!bIsSelfHost ? LobbyUIColor::Selected : LobbyUIColor::Unselected);

	SelfHostTickIcon->SetVisibility(
		bIsSelfHost ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	DedicatedTickIcon->SetVisibility(
		!bIsSelfHost ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

static FString GameModeOption(EMatchMode Mode)
{
	if (Mode == EMatchMode::Zombie)
	{
		return TEXT("?game=/Game/GameModes/BP_GM_Zombie.BP_GM_Zombie_C");
	}
	return TEXT("?game=/Game/Core/GM_Spike.GM_Spike_C");
}

void ULobbyUI::OnStartGameClicked()
{
	int NumBotTeam1 = 0;
	int NumBotTeam2 = 0;

	for (int32 i = 0; i < PlayerSlotUIs.Num(); ++i)
	{
		auto PSlot = PlayerSlotUIs.IsValidIndex(i) ? PlayerSlotUIs[i] : nullptr;
		if (PSlot && !PSlot->IsEmpty())
		{
			PlayerRoomInfo Info = PSlot->GetPlayerInfo();
		
			if (Info.bIsBot)
			{
				if (i < 5)
				{
					++NumBotTeam1;
				}
				else
				{
					++NumBotTeam2;
				}
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Starting Game with %d bots in Team 1 and %d bots in Team 2"), NumBotTeam1, NumBotTeam2);
	

	FString Options(TEXT("?listen?game=/Game/Main/Core/GM_Spike.GM_Spike_C"));

	Options += FString::Printf(TEXT("?BotT1=%d"), NumBotTeam1);
	Options += FString::Printf(TEXT("?BotT2=%d"), NumBotTeam2);
	if (CurrentRoomData.Mode == EMatchMode::Spike)
	{
		const FName MapName(TEXT("/Game/Main/Maps/GhostMallMap")); 
		UGameplayStatics::OpenLevel(this, MapName, /*bAbsolute*/ true, Options);
		return;
	}
	else if (CurrentRoomData.Mode == EMatchMode::Zombie)
	{
		const FName MapName(TEXT("/Game/Main/Maps/GhostMallMap"));
		UGameplayStatics::OpenLevel(this, MapName, /*bAbsolute*/ true, Options);
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Start Game Clicked"));
	UGameplayStatics::OpenLevel(
		this,
		FName(TEXT("Main/Maps/L_PlayGround"))
	);
}

void ULobbyUI::OnAddBotTeam1()
{
	UE_LOG(LogTemp, Warning, TEXT("Add Bot Team 1 Clicked"));
	URoomPlayerSlotUI* EmptySlot = nullptr;
	// check from 0 -> 4 for team 1
	for (int32 i = 0; i < 5; ++i)
	{
		auto PSlot = PlayerSlotUIs.IsValidIndex(i) ? PlayerSlotUIs[i] : nullptr;
		if (PSlot && PSlot->IsEmpty())
		{
			EmptySlot = PSlot;
			break;
		}
	}

	if (EmptySlot)
	{
		PlayerRoomInfo BotInfo;
		BotInfo.PlayerName = TEXT("Bot");
		BotInfo.PlayerId = 1; // assign unique id
		BotInfo.bIsBot = true;
		EmptySlot->SetPlayerInfo(BotInfo);
	}
}

void ULobbyUI::OnAddBotTeam2()
{
	UE_LOG(LogTemp, Warning, TEXT("Add Bot Team 2 Clicked"));

	URoomPlayerSlotUI* EmptySlot = nullptr;
	// check from 5 -> 9 for team 2
	for (int32 i = 5; i < 10; ++i)
	{
		auto PSlot = PlayerSlotUIs.IsValidIndex(i) ? PlayerSlotUIs[i] : nullptr;
		if (PSlot && PSlot->IsEmpty())
		{
			EmptySlot = PSlot;
			break;
		}
	}
	if (EmptySlot)
	{
		PlayerRoomInfo BotInfo;
		BotInfo.PlayerName = TEXT("Bot");
		BotInfo.PlayerId = 1; // assign unique id
		BotInfo.bIsBot = true;
		EmptySlot->SetPlayerInfo(BotInfo);
	}
}

void ULobbyUI::OnDeletePlayer(int32 PlayerId)
{
	UE_LOG(LogTemp, Warning, TEXT("Delete Player Clicked: %d"), PlayerId);
	// update current match data
	for (int32 i = 0; i < CurrentRoomData.Players.Num(); ++i)
	{
		if (CurrentRoomData.Players[i].PlayerId == PlayerId)
		{
			CurrentRoomData.Players[i].PlayerId = -1; // mark as empty
			break;
		}
	}
}

void ULobbyUI::HandleCreateRoomSuccess() {
	if (ListPn) {
		ListPn->SetVisibility(ESlateVisibility::Visible);
	}
	if (CreatePn) {
		CreatePn->SetVisibility(ESlateVisibility::Collapsed);
	}
}