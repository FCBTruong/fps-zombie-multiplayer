#include "Game/ShooterGameMode.h"
#include "Game/ShooterGameState.h"
#include "Weapons/WeaponDataManager.h"
#include "Controllers/MyPlayerController.h"
#include "Game/GameManager.h"

void AShooterGameMode::StartPlay()
{
    UE_LOG(LogTemp, Warning, TEXT("AShooterGameMode:Game Started!"));
    Super::StartPlay();

    AShooterGameState* GS = GetGameState<AShooterGameState>();
    if (!GS)
        return;

    UWeaponDataManager* WeaponDataMgr = GetGameInstance()->GetSubsystem<UWeaponDataManager>();
    if (!WeaponDataMgr || WeaponDataMgr->GetAllWeapons().Num() == 0)
        return;

    UGameManager* GMR = GetGameInstance()->GetSubsystem<UGameManager>();
    if (!GMR)
    {
        return;
    }
}

void AShooterGameMode::PostLogin(APlayerController* NewPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("Player Logged In: %s"), *GetNameSafe(NewPlayer));
    Super::PostLogin(NewPlayer);

    if (!NewPlayer->IsLocalController())
    {
        AMyPlayerController* MyPC = Cast<AMyPlayerController>(NewPlayer);
        if (MyPC)
        {
           
        }
    }
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

   
    AShooterGameState* GS = GetGameState<AShooterGameState>();
    if (GS)
    {
        GS->MulticastKillNotify(KillerPS, VictimPS, DamageCauser, bWasHeadShot);
    }
}

void AShooterGameMode::AddPlayer(APlayerController* NewPlayer)
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

    if (TeamA.Num() > TeamB.Num())
    {
        AssignedTeam = "B";
        TeamB.Add(NewPlayer);
    }
    else
    {
        AssignedTeam = "A";
        TeamA.Add(NewPlayer);
    }

    PS->SetTeamID(AssignedTeam);
}

FString AShooterGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) {

    UE_LOG(LogTemp, Warning, TEXT("InitNewPlayer called in TeamEliminationMode"));
    AddPlayer(NewPlayerController);
    return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
}

void AShooterGameMode::ResetPlayers()
{
    for (AController* PC : TeamA)
    {
        if (PC)
        {
            if (APawn* OldPawn = PC->GetPawn())
            {
                OldPawn->Destroy();     // Required!
            }
            RestartPlayer(PC);          // Will now actually respawn at a PlayerStart
        }
    }

    for (AController* PC : TeamB)
    {
        if (PC)
        {
            if (APawn* OldPawn = PC->GetPawn())
            {
                OldPawn->Destroy();
            }
            RestartPlayer(PC);
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
    NewPlayer->SetIgnoreLookInput(true);
    NewPlayer->SetIgnoreMoveInput(true);
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
    AMyPlayerState* NewPS = GetWorld()->SpawnActor<AMyPlayerState>(PlayerStateClass);
    NewPS->SetOwner(Bot);
    Bot->PlayerState = NewPS;

    NewPS->SetTeamID(TeamID);
    NewPS->SetIsAlive(true);

    // 3) Restart to spawn Pawn
    RestartPlayer(Bot);
    UE_LOG(LogTemp, Warning, TEXT("Spawned Bot for Team %s"), *TeamID.ToString());

    if (TeamID == "A")
    {
        TeamA.Add(Bot);
    }
    else if (TeamID == "B")
    {
        TeamB.Add(Bot);
	}

    return Bot;
}

bool AShooterGameMode::CheckAllTeamDead(FName TeamID)
{
    const TArray<AController*>* TeamArray = nullptr;

    if (TeamID == "A")
    {
        TeamArray = &TeamA;
    }
    else if (TeamID == "B")
    {
        TeamArray = &TeamB;
    }
    else
    {
        return false; // invalid team
    }

    // Check players
    for (AController* PC : *TeamArray)
    {
        if (!PC) continue;

        AMyPlayerState* PS = PC->GetPlayerState<AMyPlayerState>();
        if (PS && PS->IsAlive())
        {
            return false; // At least 1 alive team NOT dead
        }
    }

    // No alive players
    return true;
}


TArray<AController*> AShooterGameMode::GetTeamPlayers(FName TeamID)
{
    if (TeamID == "A")
    {
        return TeamA;
    }
    else if (TeamID == "B")
    {
        return TeamB;
    }
    return TArray<AController*>();
}