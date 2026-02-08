#include "Game/ShooterGameMode.h"
#include "Game/ShooterGameState.h"
#include "Controllers/MyPlayerController.h"
#include "Game/GameManager.h"
#include "Weapons/WeaponState.h"
#include "Characters/BaseCharacter.h"
#include "Game/ActorManager.h"
#include "Kismet/GameplayStatics.h"
#include "Lobby/RoomManager.h"
#include "Items/ItemConfig.h"
#include "Game/PlayerSlot.h"

AShooterGameMode::AShooterGameMode()
{
    bDelayedStart = true;
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
    UE_LOG(LogTemp, Warning, TEXT("DEBUGYYY -- InitGameState"));
    // use RoomData to setup game state
    URoomManager* RoomMgr = URoomManager::Get(GetWorld());
    if (!RoomMgr)
    {
        UE_LOG(LogTemp, Warning, TEXT("RoomManager is null in InitGameState"));
        return;
    }
    // get current room data
    const FRoomData& RoomData = RoomMgr->GetCurrentRoomData();

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
    for (const FPlayerRoomInfo& Player : RoomData.Players)
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

        int32 CharacterSkin = FGameConstants::SKIN_CHARACTER_ATTACKER;
        if (MatchMode == EMatchMode::Spike)
        {
            if (Idx <= 5)
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

    UE_LOG(LogTemp, Warning, TEXT("DEBUGYYY -- Startplay"));

    GS->SetMatchState(EMyMatchState::PRE_MATCH);
    GS->SetCurrentRound(0);

    int WaitingTime = 15; // seconds
    GS->SetRoundEndTime(WaitingTime);
    BotManager->SetMatchMode(GetMatchMode());

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

        ABotAIController* BotController = SpawnBot(Slot);
    }
}

void AShooterGameMode::StartMatch()
{
    Super::StartMatch();
    UE_LOG(LogTemp, Warning, TEXT("StartMatch called in AShooterGameMode"));

    // Start first round after short delay
    GetWorldTimerManager().ClearTimer(StartRoundTimerHandle);
    GetWorldTimerManager().SetTimer(
        StartRoundTimerHandle,
        this,
        &AShooterGameMode::StartRoundDelayed,
        1.0f,   // delay seconds (change as needed)
        false
    );
}

void AShooterGameMode::StartRoundDelayed()
{
    if (!HasAuthority()) return;
    if (!HasMatchStarted()) return;

    StartRound();
}

bool AShooterGameMode::ReadyToStartMatch_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("ReadyToStartMatch called in AShooterGameMode"));
    return bIsAllPlayersJoined;
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

    UE_LOG(LogTemp, Warning, TEXT("DEBUGYYY PreLogin called in AShooterGameMode"));
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
    UE_LOG(LogTemp, Warning, TEXT("DEBUGYYY Login called in AShooterGameMode"));
    auto X = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
	UE_LOG(LogTemp, Warning, TEXT("DEBUGYYY Login returned PC=%p"), static_cast<void*>(X));
    return X;
}

void AShooterGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    UE_LOG(LogTemp, Warning, TEXT("DEBUGYYY -- PostLogin"));
    AMyPlayerController* MyPC = Cast<AMyPlayerController>(NewPlayer);

    if (MyPC)
    {
        AShooterGameState* GS = GetGameState<AShooterGameState>();

		AMyPlayerState* PS = MyPC->GetPlayerState<AMyPlayerState>();

        if (!PS) {
            UE_LOG(LogTemp, Warning, TEXT("PlayerState is null in PostLogin"));
            return;
        }

        APlayerSlot* Slot = GS->GetPlayerSlot(MyPC->BackendUserId);
        if (Slot) {
            Slot->SetIsConnected(true);
        }
		PS->SetPlayerSlot(Slot);

        NewPlayer->StartSpot = nullptr;
        RestartPlayer(NewPlayer);

        if (!this->HasMatchStarted() && !bIsAllPlayersJoined)
        {
            bool bCheck = true;
            // check if all players joined
            for (const APlayerSlot* S : GS->Slots)
            {
                if (S->IsBot())
                {
                    continue;
                }

                if (!S->IsConnected())
                {
                    bCheck = false;
                    break;
                }
            }
            if (bCheck)
            {
                bIsAllPlayersJoined = true;

                // All players joined, can start match now
                UE_LOG(LogTemp, Warning, TEXT("All players joined!"));
            }
        }
    }
}

void AShooterGameMode::Logout(AController* Exiting)
{
    APawn* LeavingPawn = Exiting ? Exiting->GetPawn() : nullptr;

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
    if (KillerPS && KillerPS != VictimPS && KillerPS->GetTeamId() != VictimPS->GetTeamId()) {
        KillerPS->AddKill();
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
    UE_LOG(LogTemp, Warning, TEXT("DEBUGYYY -- InitNewPlayer"));
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
		MPC->BackendUserId = BackendPlayerId;

       /* APlayerSlot* Slot = CachedGS->GetPlayerSlot(BackendPlayerId);

        if (!Slot) {
            UE_LOG(LogTemp, Warning, TEXT("DEBUGYYY InitNewPlayer: No PlayerSlot found for PlayerId: %d"), BackendPlayerId);
        }

		AMyPlayerState* PS = MPC->GetPlayerState<AMyPlayerState>();
        if (PS)
        {
            UE_LOG(LogTemp, Warning, TEXT("DEBUGYYY PS=%p, MPC=%p, (MPC=%s, PSName=%s)"),
                static_cast<void*>(PS),
                static_cast<void*>(MPC),
                *GetNameSafe(MPC),
                *GetNameSafe(PS));
            UE_LOG(LogTemp, Warning, TEXT("DEBUGYYY InitNewPlayer: Setting PlayerId: %d"), BackendPlayerId);
            PS->SetPlayerSlot(Slot);
        }*/
    }
	return ErrorMessage;
}

void AShooterGameMode::ResetPlayers()
{
    // clean pawns first
    CleanupCorpses();
    AShooterGameState* GS = GetGameState<AShooterGameState>();

    if (!GS) {
        UE_LOG(LogTemp, Warning, TEXT("GameState is null in ResetPlayers"));
        return;
    }

    auto Slots = GS->Slots;
    for (APlayerSlot* Slot : Slots)
    {
        APawn* Pawn = Slot->GetPawn();
        if (IsValid(Pawn))
        {
            AController* Controller = Pawn->GetController();

            Pawn->Destroy();
            Pawn = nullptr;
            ;
            if (Controller) {
                Controller->StartSpot = nullptr;
                Controller->UnPossess();

                APlayerState* PS = Controller->GetPlayerState<APlayerState>();
                if (PS) {
                    PS->SetIsSpectator(false);
                }
                RestartPlayer(Controller);
            }
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

bool AShooterGameMode::CheckAllTeamDead(ETeamId TeamID)
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
            A->Destroy();
        }
    }
    Corpses.Empty();
}

void AShooterGameMode::HandleMatchHasStarted()
{
    UE_LOG(LogTemp, Warning, TEXT("Match Has Started in AShooterGameMode"));
    Super::HandleMatchHasStarted();
}

void AShooterGameMode::StartRound() {
    UE_LOG(LogTemp, Warning, TEXT("Starting Round in AShooterGameMode"));

    AActorManager* AM = AActorManager::Get(GetWorld());
    AM->ResetPlayerStartsUsage();

    AShooterGameState* GS = GetGameState<AShooterGameState>();
    GS->SetCurrentRound(GS->GetCurrentRound() + 1);
}

void AShooterGameMode::EndRound(ETeamId WinningTeam)
{

}

void AShooterGameMode::EndGame(ETeamId WinningTeam)
{
    AShooterGameState* GS = GetGameState<AShooterGameState>();
    if (GS) {
        GS->SetMatchState(EMyMatchState::GAME_ENDED);
        GS->Multicast_GameResult(WinningTeam);
    }

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
    FTimerHandle Timer;
    GetWorld()->GetTimerManager().SetTimer(
        Timer,
        this,
        &AShooterGameMode::TravelToLobby,
        1.5f,
        false
    );
}

bool AShooterGameMode::IsDamageAllowed(AController* Killer, AController* Victim) const
{
    if (true) {
        return true;
    }
    AShooterGameState* GS = GetGameState<AShooterGameState>();
    if (!GS)
        return false;

    if (GS->GetMatchState() == EMyMatchState::BUY_PHASE
        || GS->GetMatchState() == EMyMatchState::PRE_MATCH)
    {
        return false;
    }
    // not allow same team
    /*AMyPlayerState* KillerPS = Killer ? Killer->GetPlayerState<AMyPlayerState>() : nullptr;
    AMyPlayerState* VictimPS = Victim ? Victim->GetPlayerState<AMyPlayerState>() : nullptr;
    if (KillerPS && VictimPS && KillerPS->GetTeamId() == VictimPS->GetTeamId()) {
        return false;
    }*/
    return true;
}

void AShooterGameMode::TravelToLobby()
{
    UE_LOG(LogTemp, Warning, TEXT("Traveling to Lobby"));
    GetWorld()->ServerTravel(FGameConstants::LEVEL_LOBBY.ToString());
}

APawn* AShooterGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
    UE_LOG(LogTemp, Warning, TEXT("DEBUGYYY SpawnDefaultPawnAtTransform called in AShooterGameMode"));
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

    UE_LOG(LogTemp, Warning, TEXT("DEBUGYYY SPAWN PS=%p , MPC=%p ( PSName=%s)"),
        static_cast<void*>(PS),
		static_cast<void*>(NewPlayer),
        *GetNameSafe(PS));

    APlayerSlot* Slot = PS->GetPlayerSlot();

    if (!Slot)
    {
        UE_LOG(LogTemp, Warning, TEXT("DEBUGYYY No player slot for id %d"), PS->GetBackendUserId());
        return nullptr;
    }

    if (IsValid(Slot->GetPawn()))
    {
        return Slot->GetPawn();
    }
    APawn* ResultPawn = Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
    if (!ResultPawn)
    {
        return nullptr;
    }

    Slot->SetPawn(ResultPawn);

    ABaseCharacter* MyChar = Cast<ABaseCharacter>(ResultPawn);
    if (MyChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("Setting character skin for spawned pawn %d"), Slot->GetCharacterSkin());
        MyChar->SetCharacterSkin(Slot->GetCharacterSkin());
    }
    return ResultPawn;
}