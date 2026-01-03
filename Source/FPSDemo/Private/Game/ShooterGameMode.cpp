#include "Game/ShooterGameMode.h"
#include "Game/ShooterGameState.h"
#include "Controllers/MyPlayerController.h"
#include "Game/GameManager.h"
#include "Weapons/WeaponState.h"
#include "Characters/BaseCharacter.h"

AShooterGameMode::AShooterGameMode()
{
    bDelayedStart = true;
}

void AShooterGameMode::StartPlay()
{
    Super::StartPlay();
    BotManager = MakeUnique<BotStateManager>();

    UE_LOG(LogTemp, Warning, TEXT("AShooterGameMode:Game Started!"));

    AShooterGameState* GS = GetGameState<AShooterGameState>();
    if (!GS)
        return;

    bDelayedStart = true;
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
}


void AShooterGameMode::NotifyPlayerKilled(class AController* Killer, ABaseCharacter* Victim, const UItemConfig* DamageCauser, bool bWasHeadShot)
{
    UE_LOG(LogTemp, Warning, TEXT("NotifyPlayerKilled called in TeamEliminationMode"));

    AMyPlayerState* KillerPS = Killer ? Killer->GetPlayerState<AMyPlayerState>() : nullptr;
    AMyPlayerState* VictimPS = Victim ? Victim->GetPlayerState<AMyPlayerState>() : nullptr;
    if (!VictimPS) {
        UE_LOG(LogTemp, Warning, TEXT("VictimPS is null in NotifyPlayerKilled"));
        return;
    }
	VictimPS->AddDeath();
    
    // check same team or not
    if (KillerPS && KillerPS != VictimPS && KillerPS->GetTeamID() != VictimPS->GetTeamID()) {
        KillerPS->AddKill();
    }
   
    AShooterGameState* GS = GetGameState<AShooterGameState>();
    if (GS)
    {
        GS->MulticastKillNotify(KillerPS, VictimPS, DamageCauser, bWasHeadShot);
    }
}

void AShooterGameMode::AssignPlayerTeam(APlayerController* NewPlayer)
{
    UE_LOG(LogTemp, Warning, TEXT("AddPlayer called in AShooterGameMode"));
    if (!NewPlayer)
    {
        return;
    }

    AMyPlayerState* PS = NewPlayer->GetPlayerState<AMyPlayerState>();
    if (!PS) {
        UE_LOG(LogTemp, Warning, TEXT("PlayerState is null in AddPlayer"));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("Adding player to team..."));

    FName AssignedTeam;

	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (!GS) {
		UE_LOG(LogTemp, Warning, TEXT("GameState is null in AssignPlayerTeam"));
		return;
	}

    // pick suitable team
	int32 TeamACount = 0;
	int32 TeamBCount = 0;
    TArray<APlayerState*> PlayerStates = GS->PlayerArray;
    for (APlayerState* ExistingPS : PlayerStates)
    {
        if (!ExistingPS) continue;
        AMyPlayerState* MyExistingPS = Cast<AMyPlayerState>(ExistingPS);
        if (MyExistingPS && MyExistingPS->GetTeamID() == FName("A"))
        {
            TeamACount++;
        }
        else if (MyExistingPS && MyExistingPS->GetTeamID() == FName("B"))
        {
            TeamBCount++;
        }
    }
    if (TeamACount <= TeamBCount)
    {
        AssignedTeam = FName("A");
    }
    else
    {
        AssignedTeam = FName("B");
    }

	UE_LOG(LogTemp, Warning, TEXT("Assigned Team: %s"), *AssignedTeam.ToString());

    PS->SetTeamID(AssignedTeam);
}

FString AShooterGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) {

    UE_LOG(LogTemp, Warning, TEXT("InitNewPlayer called in TeamEliminationMode"));
	AssignPlayerTeam(NewPlayerController);
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
	UE_LOG(LogTemp, Warning, TEXT("DEBUGGG:::: Number of PlayerStates = %d"), PlayerStates.Num());

    for (APlayerState* PS : PlayerStates)
    {
		if (!PS) continue;
       
        AController* Controller = PS->GetOwner<AController>();
      
        if (Controller)
        {
			UE_LOG(LogTemp, Warning, TEXT("DEBUGGG::: Restarting player: %s"), *GetNameSafe(Controller));
            if (APawn* P = Controller->GetPawn())
            {
                Controller->UnPossess();
                bool R = P->Destroy();
            }
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

ABotAIController* AShooterGameMode::SpawnBot(FName TeamID)
{
    UE_LOG(LogTemp, Warning, TEXT("Spawning Bot for Team %s"), *TeamID.ToString());
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // 1) Spawn AI controller
    ABotAIController* Bot = GetWorld()->SpawnActor<ABotAIController>(ABotAIController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
    if (!Bot) return nullptr;

    // 2) Set team in PlayerState
    Bot->InitPlayerState();

	AMyPlayerState* PS = Bot->GetPlayerState<AMyPlayerState>();
	if (PS)
	{
		PS->SetTeamID(TeamID);
		FName BotName = FName(*FString::Printf(TEXT("Bot_%s_%d"), *TeamID.ToString(), FMath::RandRange(1000, 9999)));
		PS->SetPlayerName(BotName.ToString());
	}

    UE_LOG(LogTemp, Warning, TEXT("Spawned Bot for Team %s"), *TeamID.ToString());

    if (BotManager)
    {
        BotManager->AddBot(Bot);
	}
    return Bot;
}

bool AShooterGameMode::CheckAllTeamDead(FName TeamID)
{
    // Check players
    TArray<APlayerState*> PlayerStates = GetGameState<AShooterGameState>()->PlayerArray;

    for (APlayerState* PS : PlayerStates)
    {
        AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS);
        if (!MyPS) continue;

		if (MyPS->GetTeamID() != TeamID) continue;

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
    /*
    TArray<APlayerState*> PlayerStates = GS->PlayerArray;
    for (APlayerState* PS : PlayerStates)
    {
        if (!PS) continue;
        AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS);
        if (MyPS)
        {
            if (!MyPS->IsAlive()) {
				MyPS->ClearOwnedWeapons();
                continue;
            }
            // get pawn
			APawn* Pawn = MyPS->GetPawn();
            if (!Pawn) continue;
            ABaseCharacter* MyChar = Cast<ABaseCharacter>(Pawn);
            if (!MyChar) continue;
			UWeaponComponent* WeaponComp = MyChar->GetWeaponComponent();
			if (!WeaponComp) continue;
            FWeaponState* W = WeaponComp->GetRifleState();
            if (W) {
                MyPS->AddOwnedWeapon(W->ItemId);
            }
        }
    }*/
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
    StartRound();
}

bool AShooterGameMode::ReadyToStartMatch_Implementation()
{
    if (GetNumPlayers() <= 0) return false;

    return true;
}

void AShooterGameMode::StartRound() {
	UE_LOG(LogTemp, Warning, TEXT("Starting Round in AShooterGameMode"));
}