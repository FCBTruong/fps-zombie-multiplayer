#include "Game/ShooterGameMode.h"
#include "Game/ShooterGameState.h"
#include "Weapons/WeaponDataManager.h"
#include "Controllers/MyPlayerController.h"
#include "Game/GameManager.h"

void AShooterGameMode::StartPlay()
{
    UE_LOG(LogTemp, Warning, TEXT("Game Started!"));
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

    FVector Origin(0.f, 0.f, 100.f);
    float RangeX = 1000.f;
    float RangeY = 1000.f;
    TArray<FPickupData> ItemArray;

	TArray<UWeaponData*> SpawnableItems = TArray<UWeaponData*>();
	TArray<UWeaponData*> AllWeapons = WeaponDataMgr->GetAllWeapons();
    for (UWeaponData* Weapon : AllWeapons)
    {
        if (Weapon)
        {
            if (Weapon->WeaponType == EWeaponTypes::Firearm) {
                SpawnableItems.Add(Weapon);
            }
        }
	}
    if (SpawnableItems.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No spawnable items found"));
        return;
	}
    for (int32 i = 0; i < 50; i++)
    {
        float RandX = FMath::FRandRange(-RangeX, RangeX);
        float RandY = FMath::FRandRange(-RangeY, RangeY);
        float RandZ = 100.f;

        FVector SpawnLocation(RandX, RandY, RandZ);

        UWeaponData* PickupObj = SpawnableItems[FMath::RandRange(0, SpawnableItems.Num() - 1)];
        if (!PickupObj)
        {
            continue;
        }

        FPickupData P;
		P.Id = GMR->GetNextItemOnMapId();
        P.ItemId = PickupObj->Id;
        P.Amount = 1;
        P.Location = SpawnLocation;

        GS->ItemsOnMap.Add(P.Id, P);
        ItemArray.Add(P);
    }


    UWorld* World = GetWorld();
    if (World)
    {
        if (World->GetNetMode() == NM_DedicatedServer)
        {
            UE_LOG(LogTemp, Log, TEXT("Skip GenItemNodesOnMap on dedicated server"));
            return;
        }
        else {
            GMR->GenItemNodesOnMap(ItemArray);
        }
    }
    

    UE_LOG(LogTemp, Log, TEXT("Generated %d items on map"), GS->ItemsOnMap.Num());
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
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [MyPC, this]()
                {
                    AShooterGameState* GS = GetGameState<AShooterGameState>();
                    if (GS)
                    {
                        MyPC->Client_ReceiveItemsOnMap(GS->GetItemsOnMap());
                    }
                }, 0.5f, false);
        }
    }
}


void AShooterGameMode::NotifyPlayerKilled(class AController* Killer, class AController* Victim, class UWeaponData* DamageCauser, bool bWasHeatShot)
{
    
}

void AShooterGameMode::AddPlayer(APlayerController* NewPlayer)
{
    
}

FString AShooterGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) {

    UE_LOG(LogTemp, Warning, TEXT("InitNewPlayer called in TeamEliminationMode"));
    AddPlayer(NewPlayerController);
    return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
}

void AShooterGameMode::ResetPlayers()
{
    for (APlayerController* PC : TeamA)
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

    for (APlayerController* PC : TeamB)
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
    AMyPlayerState* NewPS = GetWorld()->SpawnActor<AMyPlayerState>(PlayerStateClass);
    NewPS->SetOwner(Bot);
    Bot->PlayerState = NewPS;

    NewPS->SetTeamID(TeamID);
    NewPS->SetIsAlive(true);

    // 3) Restart to spawn Pawn
    RestartPlayer(Bot);
    UE_LOG(LogTemp, Warning, TEXT("Spawned Bot for Team %s"), *TeamID.ToString());

    return Bot;
}


