// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyUI.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Network/NetworkManager.h"
#include "Lobby/RoomManager.h"
#include "Lobby/RoomSlotUI.h"
#include "Lobby/RoomSlotUI.h"

void ULobbyUI::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogTemp, Warning, TEXT("LobbyUI: NativeConstruct called"));

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI) {
		return;
	}
	if (UNetworkManager* NetworkManager =
		GI->GetSubsystem<UNetworkManager>())
	{
		CachedNetworkManager = NetworkManager;
	}
	if (URoomManager* RoomManager = GI->GetSubsystem<URoomManager>()) {
		CachedRoomMgr = RoomManager;

		RoomManager->OnRoomInfoReceived.AddUObject(this, &ULobbyUI::UpdateRoomData);
		RoomManager->OnUpdateRoomSlot.AddUObject(this, &ULobbyUI::UpdateRoomSlot);
		RoomManager->OnUpdateRoomList.AddUObject(this, &ULobbyUI::UpdateRoomList);

		RequestRoomList();
		GetWorld()->GetTimerManager().SetTimer(
			RequestRoomListTimer,
			this,
			&ULobbyUI::RequestRoomList,
			3.0f,
			true   // looping
		);
	}

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
			}
		}
	}
	ShowRoomListUI();
}

void ULobbyUI::NativeDestruct()
{
	if (CachedRoomMgr)
	{
		CachedRoomMgr->OnRoomInfoReceived.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void ULobbyUI::ShowRoomListUI()
{
	if (ListPn) {
		ListPn->SetVisibility(ESlateVisibility::Visible);
	}
	if (CreatePn) {
		CreatePn->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ULobbyUI::ShowRoomUI()
{
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
	if (!CachedRoomMgr->GetCurrentRoomData().bIsActive) {
		ShowRoomListUI();
		return;
	}

	SetMatchMode(CachedRoomMgr->GetCurrentRoomData().Mode);
	SetHostMode(CachedRoomMgr->GetCurrentRoomData().bIsSelfHost);

	for (int32 i = 0; i < PlayerSlotUIs.Num(); ++i)
	{
		const PlayerRoomInfo Info = CachedRoomMgr->GetCurrentRoomData().Players[i];

		PlayerSlotUIs[i]->SetPlayerInfo(Info, i);
	}

	ShowRoomUI();
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
	if (CachedRoomMgr->GetCurrentRoomData().Mode == EMatchMode::Spike)
	{
		const FName MapName(TEXT("/Game/Main/Maps/GhostMallMap")); 
		UGameplayStatics::OpenLevel(this, MapName, /*bAbsolute*/ true, Options);
		return;
	}
	else if (CachedRoomMgr->GetCurrentRoomData().Mode == EMatchMode::Zombie)
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
	CachedRoomMgr->RequestAddBot(URoomManager::TEAM_1);
}

void ULobbyUI::OnAddBotTeam2()
{
	CachedRoomMgr->RequestAddBot(URoomManager::TEAM_2);
}

void ULobbyUI::UpdateRoomSlot(int slotIdx) {
	URoomPlayerSlotUI* PSlot = PlayerSlotUIs[slotIdx];
	
	auto Info = CachedRoomMgr->GetCurrentRoomData().Players[slotIdx];
	PSlot->SetPlayerInfo(Info, slotIdx);
}

void ULobbyUI::RequestRoomList() {
	CachedRoomMgr->RequestRoomList();
}

void ULobbyUI::UpdateRoomList() {
	if (!ListPn) {
		return;
	}
	if (!ListPn->IsVisible()) {
		return;
	}
	if (!RoomListBox) {
		return;
	}

	if (!RoomSlotClass)
	{
		return;
	}

	RoomListBox->ClearChildren();

	const TArray<FRoomData>& Rooms = CachedRoomMgr->GetAvailableRooms();

	for (const FRoomData& Room : Rooms)
	{

		URoomSlotUI* BoxWidget =
			CreateWidget<URoomSlotUI>(GetWorld(), RoomSlotClass);

		if (!BoxWidget)
		{
			continue;
		}

		BoxWidget->Init(Room);
		RoomListBox->AddChild(BoxWidget);
	}

	UE_LOG(LogTemp, Warning, TEXT("Ulobby:UpdateRoomList"));
}