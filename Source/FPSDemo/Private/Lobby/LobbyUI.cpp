// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyUI.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Network/NetworkManager.h"
#include "Lobby/RoomManager.h"
#include "Lobby/RoomSlotUI.h"
#include "Chat/ChatUI.h"

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

		RoomManager->OnRoomInfoReceived.AddUObject(this, &ULobbyUI::UpdateRoomState);
		RoomManager->OnUpdateRoomSlot.AddUObject(this, &ULobbyUI::UpdateRoomSlot);
		RoomManager->OnUpdateRoomList.AddUObject(this, &ULobbyUI::UpdateRoomList);
		RoomManager->OnUpdateRoomOwner.AddUObject(this, &ULobbyUI::UpdateRoomOwner);
		RoomManager->OnUpdateRoomGameMode.AddUObject(this, &ULobbyUI::UpdateRoomGameMode);
		RoomManager->OnUpdateRoomHostType.AddUObject(this, &ULobbyUI::UpdateRoomHostType);

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
	if (SelfHostBtn)
	{
		SelfHostBtn->OnClicked.AddDynamic(this, &ULobbyUI::OnSelfHostClicked);
	}
	if (DedicatedServerBtn)
	{
		DedicatedServerBtn->OnClicked.AddDynamic(this, &ULobbyUI::OnDedicatedServerClicked);
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
	if (BackBtn) {
		BackBtn->OnClicked.AddDynamic(this, &ULobbyUI::OnBackClicked);
	}
	if (PracticeBtn) {
		PracticeBtn->OnClicked.AddDynamic(this, &ULobbyUI::OnPracticeBtnClicked);
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
	UpdateRoomList();

	bool bNetworkConnected = CachedNetworkManager && CachedNetworkManager->IsConnected();
	NetworkStatusActiveIcon->SetVisibility(
		bNetworkConnected ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	NetworkStatusInActiveIcon->SetVisibility(
		bNetworkConnected ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	OfflineModeTxt->SetVisibility(
		bNetworkConnected ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
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
	if (RoomPn) {
		RoomPn->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ULobbyUI::ShowRoomUI()
{
	if (ListPn) {
		ListPn->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (RoomPn) {
		RoomPn->SetVisibility(ESlateVisibility::Visible);
	}
}

void ULobbyUI::OnCreateRoomClicked() {
	if (CachedRoomMgr) {
		CachedRoomMgr->RequestCreateRoom();
	}
}

void ULobbyUI::OnBackClicked() {
	if (CachedRoomMgr &&
		CachedRoomMgr->GetCurrentRoomData().bIsActive) {
		// in room, leave room
		CachedRoomMgr->RequestLeaveRoom();
	}
}

void ULobbyUI::UpdateRoomState() {
	if (!CachedRoomMgr->GetCurrentRoomData().bIsActive) {
		ShowRoomListUI();
		return;
	}

	RoomTitleLb->SetText(
		FText::FromString(
			FString::Printf(
				TEXT("Room ID: %d"),
				CachedRoomMgr->GetCurrentRoomData().RoomId
			)
		)
	);	
	UpdateRoomGameMode();
	UpdateRoomHostType();

	for (int32 i = 0; i < PlayerSlotUIs.Num(); ++i)
	{
		UpdateRoomSlot(i);
	}

	if (CachedRoomMgr->IsMyRoom()) {
		StartBtn->SetVisibility(ESlateVisibility::Visible);
		AddBotTeam1Btn->SetVisibility(ESlateVisibility::Visible);
		AddBotTeam2Btn->SetVisibility(ESlateVisibility::Visible);
		
		ZombieModeBtn->SetIsEnabled(true);
		BombModeBtn->SetIsEnabled(true);
		SelfHostBtn->SetIsEnabled(CachedRoomMgr->GetCurrentRoomData().bEnableSelfHost);
		DedicatedServerBtn->SetIsEnabled(CachedRoomMgr->GetCurrentRoomData().bEnableDedicatedServer);
		WaitingOwnerStartTxt->SetVisibility(ESlateVisibility::Collapsed);
	}
	else {
		StartBtn->SetVisibility(ESlateVisibility::Hidden);
		AddBotTeam1Btn->SetVisibility(ESlateVisibility::Hidden);
		AddBotTeam2Btn->SetVisibility(ESlateVisibility::Hidden);

		ZombieModeBtn->SetIsEnabled(false);
		BombModeBtn->SetIsEnabled(false);
		SelfHostBtn->SetIsEnabled(false);
		DedicatedServerBtn->SetIsEnabled(false);
		WaitingOwnerStartTxt->SetVisibility(ESlateVisibility::Visible);
	}

	ShowRoomUI();
}

void ULobbyUI::OnZombieModeClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Zombie Mode Clicked"));
	CachedRoomMgr->RequestChangeGameMode(EMatchMode::Zombie);
}

void ULobbyUI::OnBombModeClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Bomb Mode Clicked"));
	CachedRoomMgr->RequestChangeGameMode(EMatchMode::Spike);
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
	CachedRoomMgr->RequestChangeHostType(true);
}

void ULobbyUI::OnDedicatedServerClicked()
{
	// start dedicated server
	CachedRoomMgr->RequestChangeHostType(false);
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
		SelfHostBtn,
		bIsSelfHost ? LobbyUIColor::Selected : LobbyUIColor::Unselected);
	SetButtonNormalColor(
		DedicatedServerBtn,
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
	if (!CachedRoomMgr)
	{
		return;
	}
	CachedRoomMgr->RequestStartGame();
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
	int OwnerId = CachedRoomMgr->GetCurrentRoomData().OwnerId;
	bool bIsGuest = !CachedRoomMgr->IsMyRoom();
	PSlot->SetPlayerInfo(Info, slotIdx, OwnerId, bIsGuest);
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

	if (Rooms.Num() == 0)
	{
		EmptyRoomTxt->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		EmptyRoomTxt->SetVisibility(ESlateVisibility::Collapsed);
	}


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

void ULobbyUI::UpdateRoomOwner() {
	UpdateRoomState();
}

void ULobbyUI::UpdateRoomHostType() {
	SetHostMode(CachedRoomMgr->GetCurrentRoomData().bIsSelfHost);
}

void ULobbyUI::UpdateRoomGameMode() {
	SetMatchMode(CachedRoomMgr->GetCurrentRoomData().Mode);
}

void ULobbyUI::OnPracticeBtnClicked()
{
	UGameplayStatics::OpenLevel(
		this,
		FName(*(FGameConstants::LEVEL_PRACTICE.ToString() + TEXT("?listen")))
	);
}

void ULobbyUI::ToggleChat()
{
	UE_LOG(LogTemp, Warning, TEXT("LobbyUI: ToggleChat called"));
	if (ChatWidget)
	{
		ChatWidget->OpenChat();
	}
}