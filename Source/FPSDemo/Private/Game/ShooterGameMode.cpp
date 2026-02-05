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

void AShooterGameMode::StartPlay()
{
    Super::StartPlay();

    UE_LOG(LogTemp, Warning, TEXT("AShooterGameMode:Game Started!"));

	// call start round after short delay
    /*GetWorldTimerManager().SetTimer(
        TryStartMatchHandle,
        this,
        &AShooterGameMode::StartRound,
        1.0f,
        false
	);*/
	
    AShooterGameState* GS = GetGameState<AShooterGameState>();
    if (!GS)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameState is null in InitGame"));
        return;
    }

    GS->SetMatchMode(GetMatchMode());
    GS->SetCurrentRound(0);
    BotManager->SetMatchMode(GetMatchMode());

    URoomManager* RoomMgr = URoomManager::Get(GetWorld());
    // get current room data
    const FRoomData& RoomData = RoomMgr->GetCurrentRoomData();

    // if is editor, dedicated server, hardcode edit it
    if (true) {
        FRoomData& RoomDataMutable = RoomMgr->GetCurrentRoomDataMutable();

        for (int i = 0; i < 10; i++) {
            PlayerRoomInfo P = {};
            P.PlayerId = FGameConstants::EMPTY_PLAYER_ID;
			RoomDataMutable.Players.Add(P);
        }
        if (true) {
            RoomDataMutable.Players[3].PlayerId = FGameConstants::BOT_PLAYER_ID_START + 2;
            RoomDataMutable.Players[3].bIsBot = true;

            RoomDataMutable.Players[6].PlayerId = FGameConstants::BOT_PLAYER_ID_START + 22;
            RoomDataMutable.Players[6].bIsBot = true;

            /*RoomDataMutable.Players[4].PlayerId = FGameConstants::BOT_PLAYER_ID_START + 22;
            RoomDataMutable.Players[4].bIsBot = true;*/

            RoomDataMutable.Players[7].PlayerId = FGameConstants::BOT_PLAYER_ID_START + 22;
            RoomDataMutable.Players[7].bIsBot = true;

            RoomDataMutable.Players[8].PlayerId = FGameConstants::BOT_PLAYER_ID_START + 22;
            RoomDataMutable.Players[8].bIsBot = true;
        }
		UE_LOG(LogTemp, Warning, TEXT("Editor mode: Added 2 bots to room data"));
    }

    int idx = 0;
    for (const PlayerRoomInfo& Player : RoomData.Players)
    {
		UE_LOG(LogTemp, Warning, TEXT("Player ID: %d, IsBot: %d"), Player.PlayerId, Player.bIsBot);
        if (Player.PlayerId == FGameConstants::EMPTY_PLAYER_ID) {
            continue;
        }
        if (!Player.bIsBot)
        {
            continue;
        }

		bool bIsTeamA = (idx < 5); // At the moment, first 5 slots are team A, last 5 slots are team B
		idx++;

        ABotAIController* BotController = SpawnBot(bIsTeamA);
        if (BotController)
        {
            //BotController->SetTeamId(Player.TeamId);
        }
    }
}

void AShooterGameMode::StartMatch()
{
    UE_LOG(LogTemp, Warning, TEXT("AShooterGameMode: StartMatch called"));
    Super::StartMatch();
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

void AShooterGameMode::PostLogin(APlayerController* NewPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("Player Logged In: %s"), *GetNameSafe(NewPlayer));
    Super::PostLogin(NewPlayer);

    if (NewPlayer && NewPlayer->PlayerState)
    {
        JoinedPlayers.AddUnique(NewPlayer->PlayerState);
    }

    if (JoinedPlayers.Num() >= 1) { // test hardcode
        bIsAllPlayersJoined = true;
    }

    // temp
	AMyPlayerState* PS = NewPlayer->GetPlayerState<AMyPlayerState>();
    if (PS) {
		UE_LOG(LogTemp, Warning, TEXT("DEBUG01: Assigning team for player: %s"), *PS->GetPlayerName());
		PS->SetTeamId(ETeamId::Attacker);
    }
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

void AShooterGameMode::AssignPlayerTeamInit(AController* NewPlayer)
{
    
}

FString AShooterGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) {

    UE_LOG(LogTemp, Warning, TEXT("InitNewPlayer called in ShooterGameMode"));
    return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
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

	TArray<APlayerState*> PlayerStates = GS->PlayerArray;
    // log size of it

    for (APlayerState* PS : PlayerStates)
    {
		if (!PS) continue;
       
        AController* Controller = PS->GetOwner<AController>();
      
		PS->SetIsSpectator(false);

        if (Controller)
        {
            if (APawn* P = Controller->GetPawn())
            {
                Controller->UnPossess();
                bool R = P->Destroy();
            }
			Controller->StartSpot = nullptr;
            RestartPlayer(Controller);
        }
	}
}

void AShooterGameMode::RestartPlayer(AController* NewPlayer)
{
    Super::RestartPlayer(NewPlayer);

    UE_LOG(LogTemp, Warning, TEXT("DEBUG01: Restart player"));
}


ABotAIController* AShooterGameMode::SpawnBot(bool IsTeamA)
{
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // 1) Spawn AI controller
    ABotAIController* Bot = GetWorld()->SpawnActor<ABotAIController>(ABotAIController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
    if (!Bot) return nullptr;

    Bot->InitPlayerState();

	AMyPlayerState* PS = Bot->GetPlayerState<AMyPlayerState>();
	if (PS)
	{
		FName BotName = FName(*FString::Printf(TEXT("Bot_%d"), FMath::RandRange(1000, 9999)));
		PS->SetPlayerName(BotName.ToString());

        if (GetMatchMode() == EMatchMode::Spike) {
            // in spike mode, team A is attacker, team B is defender
            if (IsTeamA) {
                PS->SetTeamId(ETeamId::Attacker);
            }
            else {
                PS->SetTeamId(ETeamId::Defender);
            }
        }
        else {
            
		}
	}

    if (BotManager)
    {
        BotManager->AddBot(Bot);
	}
    return Bot;
}

bool AShooterGameMode::CheckAllTeamDead(ETeamId TeamID)
{
    // Check players
    TArray<APlayerState*> PlayerStates = GetGameState<AShooterGameState>()->PlayerArray;

    for (APlayerState* PS : PlayerStates)
    {
        AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS);
        if (!MyPS) continue;

		if (MyPS->GetTeamId() != TeamID) continue;

		ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPS->GetPawn());
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

bool AShooterGameMode::ReadyToStartMatch_Implementation()
{
    if (GetNumPlayers() <= 0) return false;
	if (!bIsAllPlayersJoined) return false;
    return true;
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