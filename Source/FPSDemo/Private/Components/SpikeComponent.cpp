#include "Components/SpikeComponent.h"
#include "Characters/BaseCharacter.h"
#include "Components/InventoryComponent.h"
#include "Game/SpikeMode.h"
#include "Game/ShooterGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Game/ActorManager.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "Kismet/KismetMathLibrary.h"

USpikeComponent::USpikeComponent()
{
    SetIsReplicatedByDefault(true);
}

void USpikeComponent::RequestPlantSpike()
{
	UE_LOG(LogTemp, Log, TEXT("USpikeComponent::RequestPlantSpike called"));
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character || !Character->IsAlive())
        return;

    if (Character->IsLocallyControlled()) {
        if (CanPlantHere()) {
            UE_LOG(LogTemp, Log, TEXT("USpikeComponent::RequestPlantSpike: Can plant here"));
        }
        else {
            
            UE_LOG(LogTemp, Log, TEXT("USpikeComponent::RequestPlantSpike: Cannot plant here"));
		}
    }

    if (!Character->HasAuthority())
    {
        ServerStartPlantSpike();
        return;
    }

    StartPlant_Internal();
}

void USpikeComponent::RequestStopPlantSpike()
{
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character)
        return;

    if (!Character->HasAuthority())
    {
        ServerStopPlantSpike();
        return;
    }
	ServerStopPlantSpike();
}

void USpikeComponent::ServerStartPlantSpike_Implementation()
{
	StartPlant_Internal();
}

void USpikeComponent::StartPlant_Internal()
{
	UE_LOG(LogTemp, Log, TEXT("USpikeComponent::StartPlant_Internal called"));
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character)
        return;

    UInventoryComponent* Inventory = Character->GetInventoryComponent();
    if (!Inventory || !Inventory->HasSpike())
        return;

    if (!CanPlantHere())
        return;

    ASpikeMode* SpikeGM =
        Cast<ASpikeMode>(UGameplayStatics::GetGameMode(GetWorld()));

    if (!SpikeGM)
        return;

    // Lock player state (crouch, animation handled elsewhere)
    Character->RequestCrouch();

    // Delay planting (e.g. 3 seconds)
    GetWorld()->GetTimerManager().SetTimer(
        PlantTimerHandle,
        this,
        &USpikeComponent::FinishPlantSpike,
        3.0f,
        false
    );
}

void USpikeComponent::ServerStopPlantSpike_Implementation()
{
    GetWorld()->GetTimerManager().ClearTimer(PlantTimerHandle);

    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (Character)
    {
        Character->RequestUnCrouch();
    }
}

void USpikeComponent::FinishPlantSpike()
{
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character)
        return;

    UInventoryComponent* Inventory = Character->GetInventoryComponent();
    if (!Inventory || !Inventory->HasSpike())
        return;

    ASpikeMode* SpikeGM =
        Cast<ASpikeMode>(UGameplayStatics::GetGameMode(GetWorld()));

    if (!SpikeGM)
        return;

    FVector PlantLocation =
        Character->GetActorLocation() +
        Character->GetActorForwardVector() * 50.f;

    SpikeGM->PlantSpike(PlantLocation, Character->GetController());

    Inventory->SetHasSpike(false);

    Character->RequestUnCrouch();
}

bool USpikeComponent::CanPlantHere() const
{
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character)
        return false;

    AActorManager* ActorManager = AActorManager::Get(GetWorld());
    if (!ActorManager)
        return false;

    TArray<ATriggerBox*> BombAreas = {
        ActorManager->GetAreaBombA(),
        ActorManager->GetAreaBombB()
    };

    const FVector CharLocation = Character->GetActorLocation();

    for (ATriggerBox* Area : BombAreas)
    {
        if (!Area)
            continue;

        UBoxComponent* Box =
            Cast<UBoxComponent>(Area->GetCollisionComponent());

        if (!Box)
            continue;

        if (UKismetMathLibrary::IsPointInBox(
            CharLocation,
            Box->GetComponentLocation(),
            Box->GetScaledBoxExtent()))
        {
            return true;
        }
    }

    return false;
}
