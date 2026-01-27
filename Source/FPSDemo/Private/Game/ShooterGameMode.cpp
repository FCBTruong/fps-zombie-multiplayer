#include "Game/ShooterGameMode.h"
#include "Game/ShooterGameState.h"
#include "Controllers/MyPlayerController.h"
#include "Game/GameManager.h"
#include "Weapons/WeaponState.h"
#include "Characters/BaseCharacter.h"
#include "Game/ActorManager.h"
#include "Kismet/GameplayStatics.h"
#include "Lobby/RoomManager.h"

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
    if (GIsEditor) {
        FRoomData& RoomDataMutable = RoomMgr->GetCurrentRoomDataMutable();

        for (int i = 0; i < 10; i++) {
            PlayerRoomInfo P = {};
            P.PlayerId = FGameConstants::EMPTY_PLAYER_ID;
			RoomDataMutable.Players.Add(P);
        }
        RoomDataMutable.Players[3].PlayerId = FGameConstants::BOT_PLAYER_ID_START + 2;
        RoomDataMutable.Players[3].bIsBot = true;

        RoomDataMutable.Players[6].PlayerId = FGameConstants::BOT_PLAYER_ID_START + 22;
        RoomDataMutable.Players[6].bIsBot = true;

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

    bIsAllPlayersJoined = true;
}


void AShooterGameMode::OnCharacterKilled(class AController* Killer, ABaseCharacter* Victim, const UItemConfig* DamageCauser, bool bWasHeadShot)
{
    UE_LOG(LogTemp, Warning, TEXT("NotifyPlayerKilled called in AShooterGameMode"));

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

        AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS);
        if (MyPS)
        {
            MyPS->SetIsSpectator(false);
            MyPS->ResetBoughtItems();
        }
	}
}

void AShooterGameMode::RestartPlayer(AController* NewPlayer)
{
    Super::RestartPlayer(NewPlayer);
    ResetPlayerNewRound(NewPlayer);
}

void AShooterGameMode::ResetPlayerNewRound(AController * NewPlayer)
{
    if (!NewPlayer) return;
    if (AMyPlayerState* PS = NewPlayer->GetPlayerState<AMyPlayerState>())
    {
        PS->ResetBoughtItems();
    }
  
	AMyPlayerController* MyPC = Cast<AMyPlayerController>(NewPlayer);
    if (MyPC)
    {
        MyPC->SetPlayerPlay();
	}
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

}

void AShooterGameMode::MoveSpectatorsOffDeadPawn(APawn* DeadPawn)
{
    if (!DeadPawn) return;

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        AMyPlayerController* PC = Cast<AMyPlayerController>(It->Get());
        if (!PC) continue;

        if (!PC->IsSpectatingState()) continue;

        AActor* ViewTarget = PC->GetViewTarget();
        if (ViewTarget == DeadPawn)
        {
            PC->RequestSpectateNextPlayer();
        }
    }
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
    // not allow same team
	/*AMyPlayerState* KillerPS = Killer ? Killer->GetPlayerState<AMyPlayerState>() : nullptr;
	AMyPlayerState* VictimPS = Victim ? Victim->GetPlayerState<AMyPlayerState>() : nullptr;
    if (KillerPS && VictimPS && KillerPS->GetTeamId() == VictimPS->GetTeamId()) {
        return false;
	}*/
	return true;
}