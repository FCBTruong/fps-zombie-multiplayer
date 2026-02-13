#include "Game/Framework/ShooterGameMode.h"
#include "Game/Framework/ShooterGameState.h"
#include "Game/Framework/MyPlayerController.h"
#include "Game/GameManager.h"
#include "Game/Items/Weapons/WeaponState.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/Subsystems/ActorManager.h"
#include "Kismet/GameplayStatics.h"
#include "Modules/Lobby/RoomManager.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "Game/Framework/PlayerSlot.h"
#include "Game/Data/MatchInfo.h"
#include "Game/AI/BotAIController.h"

AShooterGameMode::AShooterGameMode()
{
    BotManager = MakeUnique<BotStateManager>();
}

void AShooterGameMode::InitGame(
    const FString& MapName,
    const FString& Options,
    FString& ErrorMessage)
{
    UE_LOG(LogTemp, Warning, TEXT("AShooterGameMode: InitGame called"));
    Super::InitGame(MapName, Options, ErrorMessage);

    AActorManager* ActorMgr = AActorManager::Get(GetWorld());
    BotManager->Initialize(ActorMgr);
}

void AShooterGameMode::InitGameState()
{
    Super::InitGameState();
    UE_LOG(LogTemp, Warning, TEXT("AShooterGameMode: InitGameState called"));

    UGameManager* GMR = UGameManager::Get(GetWorld());
    if (!GMR)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameManager is null in InitGameState"));
        return;
	}
    // get current room data
    const FMatchInfo& MatchInfo = GMR->GetCurrentMatchInfo();

    CachedGS = GetGameState<AShooterGameState>();
    if (!CachedGS)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameState is null in InitGameState"));
        return;
    }
    EMatchMode MatchMode = GetMatchMode();
    CachedGS->SetMatchMode(MatchMode);
    CachedGS->Slots.Empty();

    int Idx = 0;
    for (const FPlayerMatchInfo& Player : MatchInfo.Players)
    {
        Idx++;
        if (Player.PlayerId == FGameConstants::EMPTY_PLAYER_ID) {
            continue;
        }

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        APlayerSlot* Slot = GetWorld()->SpawnActor<APlayerSlot>(APlayerSlot::StaticClass(),
            FTransform::Identity, Params);

        Slot->SetBackendUserId(Player.PlayerId);
        Slot->SetPlayerName(Player.PlayerName);
        Slot->SetAvatar(Player.Avatar);

        int32 CharacterSkin = FGameConstants::SKIN_CHARACTER_ATTACKER;
        if (MatchMode == EMatchMode::Spike)
        {
			if (Idx <= 5) // First 5 players are in a team (hardcoded for now)
            {
                Slot->SetTeamId(ETeamId::Defender);
                CharacterSkin = FGameConstants::SKIN_CHARACTER_DEFENDER;
            }
            else
            {
                Slot->SetTeamId(ETeamId::Attacker);
                CharacterSkin = FGameConstants::SKIN_CHARACTER_ATTACKER;
            }
        }
        else if (MatchMode == EMatchMode::DeathMatch)
        {
            Slot->SetTeamId(ETeamId::None);
            CharacterSkin = FMath::RandRange(
                FGameConstants::SKIN_CHARACTER_ATTACKER,
                FGameConstants::SKIN_CHARACTER_DEFENDER
            );
		}
        else
        {
            Slot->SetTeamId(ETeamId::Soldier);
            CharacterSkin = FMath::RandRange(
                FGameConstants::SKIN_CHARACTER_ATTACKER,
                FGameConstants::SKIN_CHARACTER_DEFENDER
            );
        }

        Slot->SetCharacterSkin(CharacterSkin);
        Slot->SetIsBot(Player.bIsBot);
        Slot->SetCrosshairCode(Player.CrosshairCode);
        CachedGS->Slots.Add(Slot);
    }
}

FTransform AShooterGameMode::GetSpawnTransformForSlot(const APlayerSlot& Slot)
{
    AActorManager* AM = AActorManager::Get(GetWorld());

    const FVector RandomLoc = AM->RandomLocationOnMap();
    const FRotator RandomRot = FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f);

    FTransform SpawnTM(RandomRot, RandomLoc);
    return SpawnTM;
}

void AShooterGameMode::StartPlay()
{
    Super::StartPlay();

    UE_LOG(LogTemp, Warning, TEXT("AShooterGameMode:Game Started!"));

    AShooterGameState* GS = GetGameState<AShooterGameState>();
    if (!GS)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameState is null in InitGame"));
        return;
    }

    GS->SetMatchState(EMyMatchState::PRE_MATCH);
    GS->SetCurrentRound(0);

    BotManager->SetMatchMode(GetMatchMode());

	// check if all players are connected
	bool bAllConnected = AreAllPlayersConnected();
    if (bAllConnected) {
        StartMatch();
    }
    else {
        ScheduleMatchStart(MatchStartDelayDefault);
    }

    // should notify backend that game is ready
    UGameManager* GM = UGameManager::Get(GetWorld());
    if (GM && GM->DsClient.IsValid()) {
        GM->DsClient->NotifyReady([this](bool bOk, const FString& ResponseBody)
            {
            });
    }

    // Spawn Bot
    for (APlayerSlot* Slot : GS->Slots)
    {
        if (!Slot->IsBot())
        {
            continue;
        }
        Slot->SetIsConnected(true);

        ABotAIController* BotController = SpawnBot(Slot);
        Slot->SetController(BotController);
    }
}

void AShooterGameMode::ScheduleMatchStart(int DelaySeconds)
{
    GetWorldTimerManager().ClearTimer(MatchStartCountdownHandle);

    GetWorldTimerManager().SetTimer(
        MatchStartCountdownHandle,
        this,
        &AShooterGameMode::StartMatchFromCountdown,
        DelaySeconds,
        false
    );

    // keep UI countdown in sync
    if (AShooterGameState* GS = GetGameState<AShooterGameState>())
    {
        GS->SetRoundRemainingTime(DelaySeconds);
    }
}

void AShooterGameMode::StartMatchFromCountdown()
{
	UE_LOG(LogTemp, Warning, TEXT("DEBUGYYY Starting match from countdown"));
    StartMatch(); // this bypasses ReadyToStartMatch_Implementation
}

void AShooterGameMode::StartMatch()
{
    UE_LOG(LogTemp, Warning, TEXT("StartMatch called in AShooterGameMode"));
    if (bMatchStarted) {
		UE_LOG(LogTemp, Warning, TEXT("Match already started, ignoring StartMatch call"));
        return;
    }

	bMatchStarted = true;
    StartRound();
}

void AShooterGameMode::StartRoundDelayed()
{
	if (!bMatchStarted) return;
    StartRound();
}

void AShooterGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    BotManager.Reset();
    Super::EndPlay(EndPlayReason);
}

void AShooterGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
    Super::HandleStartingNewPlayer_Implementation(NewPlayer);
    UE_LOG(LogTemp, Warning, TEXT("HandleStartingNewPlayer called in AShooterGameMode"));
}

// This function is first step when a player tries to join the server
// if we set ErrorMessage to non-empty, the login will be rejected
void AShooterGameMode::PreLogin(
    const FString& Options,
    const FString& Address,
    const FUniqueNetIdRepl& UniqueId,
    FString& ErrorMessage) {
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

    if (!ErrorMessage.IsEmpty())
    {
        return; // base already rejected
    }

    // parse options to get PlayerSessionId
    const FString PlayerSessionId = UGameplayStatics::ParseOption(Options, TEXT("PlayerSessionId"));

    if (PlayerSessionId.IsEmpty())
    {
        ErrorMessage = TEXT("Missing PlayerSessionId");
        return;
    }

    int BackendUserId = 0;
    if (IsRunningDedicatedServer())
    {
        UE_LOG(LogTemp, Warning, TEXT("PreLogin: Validating PlayerSessionId: %s"), *PlayerSessionId);

        // use AWS GameLift to validate the session id

        // TODO: AcceptPlayerSession(PlayerSessionId)
    }
    else {
        // selfhost editor, always true
        BackendUserId = FCString::Atoi(*PlayerSessionId);
    }
}

APlayerController* AShooterGameMode::Login(
    UPlayer* NewPlayer,
    ENetRole InRemoteRole,
    const FString& Portal,
    const FString& Options,
    const FUniqueNetIdRepl& UniqueId,
    FString& ErrorMessage)
{
    auto X = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
    return X;
}

void AShooterGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    AMyPlayerController* MyPC = Cast<AMyPlayerController>(NewPlayer);

    if (MyPC)
    {
        AShooterGameState* GS = GetGameState<AShooterGameState>();
        if (!GS)
        {
            UE_LOG(LogTemp, Warning, TEXT("GameState is null in PostLogin"));
            return;
		}

		EMyMatchState MatchState = GS->GetMatchState();
        if (MatchState == EMyMatchState::GAME_ENDED)
        {
            return;
		}

        AMyPlayerState* PS = MyPC->GetPlayerState<AMyPlayerState>();

        if (!PS) {
            UE_LOG(LogTemp, Warning, TEXT("PlayerState is null in PostLogin"));
            return;
        }

        APlayerSlot* Slot = GS->GetPlayerSlot(MyPC->GetBackendUserId());
        if (Slot) {
            Slot->SetIsConnected(true);
            Slot->SetController(NewPlayer);
        }
        PS->SetPlayerSlot(Slot);

        /*  NewPlayer->StartSpot = nullptr;
          RestartPlayer(NewPlayer);*/
        if (!bMatchStarted)
        {
			bool bAllConnected = AreAllPlayersConnected();
            if (bAllConnected)
            {
                // Reduce remaining countdown to 3 seconds (only if currently longer)
                const float Remaining = GetWorldTimerManager().IsTimerActive(MatchStartCountdownHandle)
                    ? GetWorldTimerManager().GetTimerRemaining(MatchStartCountdownHandle)
                    : 0.f;

                if (Remaining > MatchStartDelayWhenAllJoined)
                {
                    ScheduleMatchStart(MatchStartDelayWhenAllJoined);
                }
            }
        }
		//RestartPlayer(NewPlayer);
        if (!bMatchStarted) {
			RestartPlayer(MyPC);
        }
        else {
			APawn* Pawn = Slot->GetPawn();
			ABaseCharacter* Character = Cast<ABaseCharacter>(Pawn);
            if (IsValid(Character) && !Character->IsPermanentDead())
            {
                MyPC->Possess(Slot->GetPawn());
            }
            else {
				// change to spectator        
                PS->SetIsSpectator(true);
				MyPC->RequestSpectateNextPlayer();
            }
        }
    }
}

void AShooterGameMode::Logout(AController* Exiting)
{
    if (!CachedGS) {
        return;
    }
    APawn* LeavingPawn = Exiting ? Exiting->GetPawn() : nullptr;

    AMyPlayerController* MyPC = Cast<AMyPlayerController>(Exiting);
    if (MyPC)
    {
        APlayerSlot* Slot = CachedGS->GetPlayerSlot(MyPC->GetBackendUserId());
        if (Slot) {
            Slot->SetIsConnected(false);
        }
    }

    // Unpossess before calling Super (controller cleanup happens there)
    if (LeavingPawn)
    {
        Exiting->UnPossess();
        LeavingPawn->SetOwner(nullptr);
    }
    Super::Logout(Exiting);
    UE_LOG(LogTemp, Warning, TEXT("Logout called in AShooterGameMode"));
}

void AShooterGameMode::HandleCharacterKilled(class AController* Killer, const TArray<TWeakObjectPtr<AController>>& Assists, ABaseCharacter* Victim, const UItemConfig* DamageCauser, bool bWasHeadShot)
{
    UE_LOG(LogTemp, Warning, TEXT("NotifyPlayerKilled called in AShooterGameMode"));
    if (!DamageCauser) {
        UE_LOG(LogTemp, Warning, TEXT("DamageCauser is null in NotifyPlayerKilled"));
        return;
	}
    if (DamageCauser->Id == EItemId::SPIKE) {
        Killer = nullptr; // spike planting is not counted as kill
        UE_LOG(LogTemp, Warning, TEXT("DamageCauser is None in NotifyPlayerKilled"));
    }

    AMyPlayerState* KillerPS = Killer ? Killer->GetPlayerState<AMyPlayerState>() : nullptr;
    AMyPlayerState* VictimPS = Victim ? Victim->GetPlayerState<AMyPlayerState>() : nullptr;
    if (!VictimPS) {
        UE_LOG(LogTemp, Warning, TEXT("VictimPS is null in NotifyPlayerKilled"));
        return;
    }
    VictimPS->AddDeath();

    // check same team or not
    if (KillerPS && KillerPS != VictimPS) {
		if (KillerPS->GetTeamId() == ETeamId::None ||
            (KillerPS->GetTeamId() != VictimPS->GetTeamId())) {
            KillerPS->AddKill();
        }
    }

    // Add assists score
    for (TWeakObjectPtr<AController> AssistController : Assists)
    {
        AMyPlayerState* AssistPS = AssistController.IsValid() ? AssistController->GetPlayerState<AMyPlayerState>() : nullptr;
        if (AssistPS && AssistPS != KillerPS && AssistPS != VictimPS)
        {
            AssistPS->AddAssist();
        }
    }

    AShooterGameState* GS = GetGameState<AShooterGameState>();
    if (GS)
    {
        GS->MulticastKillNotify(KillerPS, VictimPS, DamageCauser, bWasHeadShot);
    }
}

FString AShooterGameMode::InitNewPlayer(
    APlayerController* NewPlayerController,
    const FUniqueNetIdRepl& UniqueId,
    const FString& Options,
    const FString& Portal)
{
    FString ErrorMessage;
    ErrorMessage = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
    const FString PlayerSessionId = UGameplayStatics::ParseOption(Options, TEXT("PlayerSessionId"));
    AMyPlayerController* MPC = Cast<AMyPlayerController>(NewPlayerController);
    if (MPC)
    {
        int BackendPlayerId = 0;
        // if dedicated server, get player id from aws gamelift
        if (IsRunningDedicatedServer())
        {
            // TODO
        }
        else {
            // selfhost editor, use PlayerSessionId as PlayerId
            BackendPlayerId = FCString::Atoi(*PlayerSessionId);
        }
        MPC->SetBackendUserId(BackendPlayerId);
    }
    return ErrorMessage;
}

void AShooterGameMode::RestartAllPlayers()
{
    AShooterGameState* GS = GetGameState<AShooterGameState>();

    if (!GS) {
        UE_LOG(LogTemp, Warning, TEXT("GameState is null in ResetPlayers"));
        return;
    }

    auto Slots = GS->Slots;
    for (APlayerSlot* Slot : Slots)
    {
        APawn* Pawn = Slot->GetPawn();
        AController* Controller = Slot->GetController();

        if (IsValid(Pawn))
        {
            Pawn->Destroy();
            Pawn = nullptr;
			Slot->SetPawn(nullptr); 
        }

        if (Controller) {
            Controller->StartSpot = nullptr;
            Controller->UnPossess();

            APlayerState* PS = Controller->GetPlayerState<APlayerState>();
            if (PS) {
                PS->SetIsSpectator(false);
            }
            UE_LOG(LogTemp, Warning, TEXT("Restarting player controller %s"), *Controller->GetName());
            RestartPlayer(Controller);
        }
    }
}

void AShooterGameMode::RestartPlayer(AController* NewPlayer)
{
    Super::RestartPlayer(NewPlayer);
}


ABotAIController* AShooterGameMode::SpawnBot(APlayerSlot* Slot)
{
    // get player state from game state
    AShooterGameState* GS = GetGameState<AShooterGameState>();
    if (!GS) return nullptr;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // 1) Spawn AI controller
    ABotAIController* Bot = GetWorld()->SpawnActor<ABotAIController>(ABotAIController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
    if (!Bot) return nullptr;

    Bot->InitPlayerState();
    AMyPlayerState* BotPS = Bot->GetPlayerState<AMyPlayerState>();
    if (BotPS) {
        BotPS->SetPlayerSlot(Slot);
    }

    if (BotManager)
    {
        BotManager->AddBot(Bot);
    }

    RestartPlayer(Bot);
    return Bot;
}

bool AShooterGameMode::CheckAllTeamDead(ETeamId TeamID) const
{
    if (!CachedGS) {
        return false;
    }
    for (APlayerSlot* Slot : CachedGS->Slots)
    {
        if (!Slot) continue;

        if (Slot->GetTeamId() != TeamID) continue;

        APawn* Pawn = Slot->GetPawn();
        if (!IsValid(Pawn)) continue;
        ABaseCharacter* MyChar = Cast<ABaseCharacter>(Pawn);
        if (!MyChar) continue;

        if (MyChar->IsAlive())
        {
            return false; // At least 1 alive team NOT dead
        }
    }

    // No alive players
    return true;
}

void AShooterGameMode::AutoBuyForBots() {
    /* for (ABotAIController* Bot : BotControllers)
     {
         if (!Bot) continue;
         AMyPlayerState* PS = Bot->GetPlayerState<AMyPlayerState>();
         if (!PS) continue;
         PS->AutoBuy();
     }*/
}

void AShooterGameMode::SavePlayersGunsForNextRound()
{
    AShooterGameState* GS = GetGameState<AShooterGameState>();

}

AShooterGameState* AShooterGameMode::GetShooterGS() const
{
    return GetGameState<AShooterGameState>();
}

void AShooterGameMode::RegisterCorpse(AActor* Corpse)
{
    Corpses.Add(Corpse);
}

void AShooterGameMode::CleanupCorpses()
{
    for (auto& W : Corpses)
    {
        if (AActor* A = W.Get())
        {
            if (IsValid(A)) {
                A->Destroy();
            }
        }
    }
    Corpses.Empty();
}

void AShooterGameMode::StartRound() {
    UE_LOG(LogTemp, Warning, TEXT("Starting Round in AShooterGameMode"));

    AActorManager* AM = AActorManager::Get(GetWorld());
    if (!AM) {
        UE_LOG(LogTemp, Warning, TEXT("ActorManager is null in StartRound"));
        return;
    }
    AM->ResetPlayerStartsUsage();

    AShooterGameState* GS = GetGameState<AShooterGameState>();
    GS->SetCurrentRound(GS->GetCurrentRound() + 1);
}

void AShooterGameMode::EndRound(ETeamId WinningTeam)
{
    UE_LOG(LogTemp, Warning, TEXT("Ending Round in AShooterGameMode"));
}

void AShooterGameMode::EndGame(ETeamId WinningTeam)
{
    UE_LOG(LogTemp, Warning, TEXT(" "));
    AShooterGameState* GS = GetGameState<AShooterGameState>();

    if (!GS) {
        UE_LOG(LogTemp, Warning, TEXT("GameState is null in EndGame"));
        return;
    }

    if (GS->GetMatchState() == EMyMatchState::GAME_ENDED) {
        return; // already ended
    }

    GS->SetMatchState(EMyMatchState::GAME_ENDED);
    GS->Multicast_GameResult(WinningTeam);

    UGameManager* GM = UGameManager::Get(GetWorld());
    if (GM && GM->DsClient.IsValid()) {
        FString ResultJsonString = TEXT("{}");
        GM->DsClient->NotifyFinish(
            ResultJsonString,
            [this](bool bOk, const FString& ResponseBody)
            {
                // ...
            }
        );
    }

    // Delay so clients see end screen
   /* FTimerHandle Timer;
    GetWorld()->GetTimerManager().SetTimer(
        Timer,
        this,
        &AShooterGameMode::TravelToLobby,
        1.5f,
        false
    );*/
}

bool AShooterGameMode::IsDamageAllowed(AController* Killer, AController* Victim) const
{
    AShooterGameState* GS = GetGameState<AShooterGameState>();
    if (!GS)
        return false;

    if (GS->GetMatchState() == EMyMatchState::BUY_PHASE
        || GS->GetMatchState() == EMyMatchState::PRE_MATCH)
    {
        return false;
    }
    
    if (!bAllowFriendlyFire) {
        AMyPlayerState* KillerPS = Killer ? Killer->GetPlayerState<AMyPlayerState>() : nullptr;
        AMyPlayerState* VictimPS = Victim ? Victim->GetPlayerState<AMyPlayerState>() : nullptr;
        if (KillerPS && VictimPS && KillerPS->GetTeamId() == VictimPS->GetTeamId()) {
            return false;
        }
    }
    return true;
}

void AShooterGameMode::TravelToLobby()
{
    UE_LOG(LogTemp, Warning, TEXT("Traveling to Lobby"));

    const FString Url = FGameConstants::LEVEL_LOBBY.ToString();          // e.g. "/Game/Main/Levels/LEVEL_LOBBY"
    const FString Short = FPackageName::GetShortName(Url);               // "LEVEL_LOBBY"

    if (GetNetMode() == NM_ListenServer || GetNetMode() == NM_DedicatedServer)
        GetWorld()->ServerTravel(Url + TEXT("?listen"), true);
    else
        UGameplayStatics::OpenLevel(this, FName(*Short));
}

APawn* AShooterGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
    // try to get from player slot first
    AShooterGameState* GS = GetGameState<AShooterGameState>();
    if (!GS)
    {
        return nullptr;
    }

    if (!NewPlayer)
    {
        return nullptr;
    }

    AMyPlayerState* PS = NewPlayer->GetPlayerState<AMyPlayerState>();

    if (!PS)
    {
        return nullptr;
    }

    APlayerSlot* Slot = PS->GetPlayerSlot();

    if (!Slot)
    {
        UE_LOG(LogTemp, Warning, TEXT("No player slot for id %d"), PS->GetBackendUserId());
        return nullptr;
    }

    APawn* ResultPawn = Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
	Slot->SetPawn(ResultPawn);
    ABaseCharacter* MyChar = Cast<ABaseCharacter>(ResultPawn);
    if (MyChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("Setting character skin for spawned pawn %d"), Slot->GetCharacterSkin());
        MyChar->SetCharacterSkin(Slot->GetCharacterSkin());
    }
    return ResultPawn;
}

bool AShooterGameMode::AreAllPlayersConnected() const
{
    if (!CachedGS) {
        return false;
    }
    for (const APlayerSlot* S : CachedGS->Slots)
    {
        if (S->IsBot())
        {
            continue;
        }
        if (!S->IsConnected())
        {
            return false;
        }
    }
    return true;
}