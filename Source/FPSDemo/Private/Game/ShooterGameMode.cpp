#include "Game/ShooterGameMode.h"
#include "Game/ShooterGameState.h"
#include "Weapons/WeaponDataManager.h"
#include "Controllers/MyPlayerController.h"
#include "Game/GameManager.h"
#include "Weapons/WeaponState.h"
#include "Characters/BaseCharacter.h"
#include "Components/WeaponComponent.h"

void AShooterGameMode::StartPlay()
{
    Super::StartPlay();

    UE_LOG(LogTemp, Warning, TEXT("AShooterGameMode:Game Started!"));

    AShooterGameState* GS = GetGameState<AShooterGameState>();
    if (!GS)
        return;

    UWeaponDataManager* WeaponDataMgr = GetGameInstance()->GetSubsystem<UWeaponDataManager>();
    if (!WeaponDataMgr || WeaponDataMgr->GetAllWeapons().Num() == 0)
        return;
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


void AShooterGameMode::NotifyPlayerKilled(class AController* Killer, class AController* Victim, class UWeaponData* DamageCauser, bool bWasHeadShot)
{
    UE_LOG(LogTemp, Warning, TEXT("NotifyPlayerKilled called in TeamEliminationMode"));

    AMyPlayerState* KillerPS = Killer ? Killer->GetPlayerState<AMyPlayerState>() : nullptr;
    AMyPlayerState* VictimPS = Victim ? Victim->GetPlayerState<AMyPlayerState>() : nullptr;
    if (!VictimPS) {
        UE_LOG(LogTemp, Warning, TEXT("VictimPS is null in NotifyPlayerKilled"));
        return;
    }
    VictimPS->SetIsAlive(false);
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
    UE_LOG(LogTemp, Warning, TEXT("AddPlayer called in TeamEliminationMode"));
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

void AShooterGameMode::CleanPawnsOnMap()
{
    // Destroy all currently possessed pawns (players + bots)
    for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
    {
        AController* C = It->Get();
        if (!C) continue;
        if (APawn* P = C->GetPawn())
        {
            C->UnPossess();
            P->Destroy();
        }
    }
}

void AShooterGameMode::ResetPlayers()
{
    AShooterGameState* GS = GetGameState<AShooterGameState>();

    if (!GS) {
		UE_LOG(LogTemp, Warning, TEXT("GameState is null in ResetPlayers"));
		return;
    }

	TArray<APlayerState*> PlayerStates = GS->PlayerArray;

    for (APlayerState* PS : PlayerStates)
    {
		if (!PS) continue;
        AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS);
        if (MyPS)
        {
            MyPS->SetIsAlive(true);
            MyPS->SetIsSpectator(false);
            MyPS->ResetBoughtItems();
        }
       
        AController* Controller = PS->GetOwner<AController>();
      
        if (Controller)
        {
            RestartPlayer(Controller);
        }
	}
}

void AShooterGameMode::RestartPlayer(AController* NewPlayer)
{
    Super::RestartPlayer(NewPlayer);
    if (AMyPlayerState* PS = NewPlayer->GetPlayerState<AMyPlayerState>())
    {
        PS->SetIsAlive(true);
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
		PS->SetIsAlive(true);
		FName BotName = FName(*FString::Printf(TEXT("Bot_%s_%d"), *TeamID.ToString(), FMath::RandRange(1000, 9999)));
		PS->SetPlayerName(BotName.ToString());
	}

    UE_LOG(LogTemp, Warning, TEXT("Spawned Bot for Team %s"), *TeamID.ToString());
    BotControllers.Add(Bot);
    RestartPlayer(Bot);

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

        if (MyPS->IsAlive())
        {
            return false; // At least 1 alive team NOT dead
        }
    }

    // No alive players
    return true;
}

void AShooterGameMode::AutoBuyForBots() {
    for (ABotAIController* Bot : BotControllers)
    {
        if (!Bot) continue;
        AMyPlayerState* PS = Bot->GetPlayerState<AMyPlayerState>();
        if (!PS) continue;
		PS->AutoBuy();
    }
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