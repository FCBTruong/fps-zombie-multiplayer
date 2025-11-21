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
    if (!WeaponDataMgr || WeaponDataMgr->WeaponList.Num() == 0)
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
    for (int32 i = 0; i < 50; i++)
    {
        float RandX = FMath::FRandRange(-RangeX, RangeX);
        float RandY = FMath::FRandRange(-RangeY, RangeY);
        float RandZ = 100.f;

        FVector SpawnLocation(RandX, RandY, RandZ);

        UWeaponData* PickupObj = WeaponDataMgr->WeaponList[FMath::RandRange(0, WeaponDataMgr->WeaponList.Num() - 1)];
        if (!PickupObj)
            continue;

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


void AShooterGameMode::NotifyPlayerKilled(class AController* Killer, class AController* Victim, class AActor* DamageCauser)
{
    
}