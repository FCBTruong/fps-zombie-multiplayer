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
#include "Components/ActionStateComponent.h"
#include "Components/EquipComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

USpikeComponent::USpikeComponent()
{
    SetIsReplicatedByDefault(true);
}

void USpikeComponent::BeginPlay()
{
    Super::BeginPlay();
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (Character)
    {
        InventoryComp = Character->GetInventoryComponent();
        ActionStateComp = Character->GetActionStateComponent();
		EquipComp = Character->GetEquipComponent();
    }
}

void USpikeComponent::RequestPlantSpike()
{
    if (!IsEnabled()) {
        UE_LOG(LogTemp, Log, TEXT("USpikeComponent::RequestPlantSpike called - not enabled"));
        return;
    }
	UE_LOG(LogTemp, Log, TEXT("USpikeComponent::RequestPlantSpike called"));
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character || !Character->IsAlive())
        return;

    if (!InventoryComp || !InventoryComp->HasSpike())
		return;

	if (!ActionStateComp || !ActionStateComp->CanPlantNow())
		return;

    if (Character->IsLocallyControlled()) {
        if (CanPlantHere()) {
            UE_LOG(LogTemp, Log, TEXT("USpikeComponent::RequestPlantSpike: Can plant here"));
        }
        else {
			OnNotifyToastMessage.Broadcast(FText::FromString("Cannot plant spike here! Move to a bomb site."));
            UE_LOG(LogTemp, Log, TEXT("USpikeComponent::RequestPlantSpike: Cannot plant here"));
            return;
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
    if (!IsEnabled()) {
        return;
    }
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character)
        return;

    if (!ActionStateComp)
		return;

    if (ActionStateComp->GetState() != EActionState::Planting) {
        return;
    }

    if (!Character->HasAuthority())
    {
        ServerStopPlantSpike();
        return;
    }
	StopPlant_Internal();
}

void USpikeComponent::ServerStartPlantSpike_Implementation()
{
    if (!IsEnabled()) {
        return;
    }
	StartPlant_Internal();
}


// this function should be called only on server
void USpikeComponent::StartPlant_Internal()
{
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character)
        return;

    if (!IsEnabled()) {
        return;
    }

    if (!InventoryComp || !ActionStateComp || !EquipComp)
		return;

    if (EquipComp->GetActiveItemId() != EItemId::SPIKE) {
        return;
    }

    if (!ActionStateComp->CanPlantNow())
		return;

	bool Result = ActionStateComp->TrySetState(EActionState::Planting);
    if (!Result)
		return;

	UE_LOG(LogTemp, Log, TEXT("USpikeComponent::StartPlant_Internal called"));

    UInventoryComponent* Inventory = Character->GetInventoryComponent();
    if (!Inventory || !Inventory->HasSpike())
        return;

    if (!CanPlantHere())
        return;

    ASpikeMode* SpikeGM =
        Cast<ASpikeMode>(UGameplayStatics::GetGameMode(GetWorld()));

    if (!SpikeGM)
        return;

    LockMovement();

    MulticastStartPlantSpike();

	UE_LOG(LogTemp, Log, TEXT("USpikeComponent::StartPlant_Internal: Starting to plant spike"));
    GetWorld()->GetTimerManager().ClearTimer(PlantTimerHandle);
    GetWorld()->GetTimerManager().SetTimer(
        PlantTimerHandle,
        this,
        &USpikeComponent::FinishPlantSpike,
        3.0f,
        false
    );
}

void USpikeComponent::StopPlant_Internal()
{
    if (!IsEnabled()) {
        return;
    }
    if (!ActionStateComp) {
        return;
    }

    if (!ActionStateComp->IsInState(EActionState::Planting)) {
        return;
	}

    UE_LOG(LogTemp, Log, TEXT("USpikeComponent::StopPlant_Internal called"));
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character)
		return;
   
	UnlockMovement();
    
	ActionStateComp->TrySetState(EActionState::Idle);
    MulticastStopPlantSpike();

	GetWorld()->GetTimerManager().ClearTimer(PlantTimerHandle);


	OnUpdatePlantSpikeState.Broadcast(false);
}

void USpikeComponent::ServerStopPlantSpike_Implementation()
{
    if (!IsEnabled()) {
        return;
    }
	StopPlant_Internal();
}

void USpikeComponent::FinishPlantSpike()
{
    if (!IsEnabled()) {
        return;
    }
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character)
    {
        return;
    }

    if (!ActionStateComp || !InventoryComp) {
        return;
    }

    if (!ActionStateComp->IsInState(EActionState::Planting)) {
        return;
    }

    if (!InventoryComp->HasSpike())
    {
        return;
    }

	UnlockMovement();

    ASpikeMode* SpikeGM =
        Cast<ASpikeMode>(UGameplayStatics::GetGameMode(GetWorld()));

    if (!SpikeGM)
    {
        return;
    }

    FVector PlantLocation =
        Character->GetActorLocation() +
        Character->GetActorForwardVector() * 20.f;
    PlantLocation.Z -= Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - 30;

    SpikeGM->PlantSpike(PlantLocation, Character->GetController());

    InventoryComp->SetHasSpike(false);
	ActionStateComp->TrySetState(EActionState::Idle);
	// auto select next weapon

    if (EquipComp) {
        EquipComp->AutoSelectBestWeapon();
    }
}

bool USpikeComponent::CanPlantHere() const
{
  /*  if (true) {
        return true;
    }*/
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character)
        return false;

    if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
    {
        if (!MoveComp->IsMovingOnGround())
			return false;
        if (MoveComp->IsFalling()) {
			return false;
        }
    }
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

void USpikeComponent::MulticastStartPlantSpike_Implementation()
{
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (Character)
    {
        Character->OnPlantSpikeStarted();
    }
	OnUpdatePlantSpikeState.Broadcast(true);
}

void USpikeComponent::MulticastStopPlantSpike_Implementation()
{
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (Character)
    {
        Character->OnPlantSpikeStopped();
    }
    OnUpdatePlantSpikeState.Broadcast(false);
}

void USpikeComponent::LockMovement()
{
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (Character)
    {
        if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
        {
            bCachedJumpAllowed = MoveComp->IsJumpAllowed();

            MoveComp->DisableMovement();
            MoveComp->StopMovementImmediately();
            MoveComp->SetJumpAllowed(false);
        }
    }
}

void USpikeComponent::UnlockMovement()
{
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (Character)
    {
        if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
        {   
            MoveComp->SetMovementMode(MOVE_Walking);
			MoveComp->SetJumpAllowed(bCachedJumpAllowed);
        }
    }
}

void USpikeComponent::RequestStartDefuseSpike() {
    if (!IsEnabled()) {
        return;
    }
    ServerStartDefuseSpike();
}

void USpikeComponent::RequestStopDefuseSpike() {
    if (!IsEnabled()) {
        return;
    }
    ServerStopDefuseSpike();
}

void USpikeComponent::StartDefuse_Internal() {
    if (!IsEnabled()) {
        return;
    }
    ASpikeMode* SpikeGM = Cast<ASpikeMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (!SpikeGM) {
        UE_LOG(LogTemp, Warning, TEXT("ServerStartDefuseSpike: No SpikeGM found"));
        return;
    }
    AShooterGameState* GameState = Cast<AShooterGameState>(GetWorld()->GetGameState());
    if (!GameState) {
        return;
    }
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character) {
        return;
    }
    AMyPlayerState* MyPS = Cast<AMyPlayerState>(Character->GetPlayerState());
    if (!MyPS) {
        return;
	}
    if (MyPS->GetTeamId() == ETeamId::Attacker) {
        UE_LOG(LogTemp, Warning, TEXT("ServerStartDefuseSpike: Attackers cannot defuse spike"));
        return; // attackers cannot defuse
    }

    ASpike* SpikeActor = SpikeGM->GetPlantedSpike();
    if (GameState->GetMatchState() != EMyMatchState::SPIKE_PLANTED) {
        return; // can only defuse during playing state
    }
    if (!SpikeActor) {
        UE_LOG(LogTemp, Warning, TEXT("ServerStartDefuseSpike: No planted spike found"));
        return;
    }

    if (SpikeActor->IsDefuseInProgress()) {
        UE_LOG(LogTemp, Warning, TEXT("ServerStartDefuseSpike: Defuse already in progress"));
        return;
    }


    if (SpikeActor->IsDefused()) {
        UE_LOG(LogTemp, Warning, TEXT("ServerStartDefuseSpike: Spike is already defused"));
        return;
    }

	// check distance to spike
	FVector SpikeLocation = SpikeActor->GetActorLocation();
	FVector CharacterLocation = Character->GetActorLocation();
	float Distance = FVector::Dist(SpikeLocation, CharacterLocation);
	if (Distance > 200.f) {
		UE_LOG(LogTemp, Warning, TEXT("ServerStartDefuseSpike: Too far from spike to defuse"));
		return;
	}

    if (!ActionStateComp) {
        return;
    }
    if (!ActionStateComp->CanDefuseNow()) {
        return;
    }
    ActionStateComp->TrySetState(EActionState::Defusing);
	LockMovement();

    MulticastStartDefuseSpike();
    SpikeActor->StartDefuse(this);
}

void USpikeComponent::StopDefuse_Internal() {
    if (!IsEnabled()) {
        return;
    }
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character) {
        return;
    }
    ASpikeMode* SpikeGM = Cast<ASpikeMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (!SpikeGM) {
        UE_LOG(LogTemp, Warning, TEXT("ServerStopDefuseSpike: No SpikeGM found"));
        return;
    }
    if (!ActionStateComp) {
        return;
    }

    if (!ActionStateComp->IsInState(EActionState::Defusing)) {
        return;
    }
    ASpike* SpikeActor = SpikeGM->GetPlantedSpike();
    if (!SpikeActor) {
        return;
    }
    if (!SpikeActor->IsDefuseInProgress()) {
        return;
    }
    
    SpikeActor->CancelDefuse();
    ActionStateComp->TrySetState(EActionState::Idle);
    UnlockMovement();
    
    MulticastStopDefuseSpike();
}

void USpikeComponent::ServerStartDefuseSpike_Implementation() {
    if (!IsEnabled()) {
        return;
    }
    StartDefuse_Internal();
}

void USpikeComponent::ServerStopDefuseSpike_Implementation() {
    if (!IsEnabled()) {
        return;
    }
    StopDefuse_Internal();
}

void USpikeComponent::MulticastStartDefuseSpike_Implementation() {
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character) {
        return;
	}
    OnUpdateDefuseSpikeState.Broadcast(true);
    Character->OnDefuseSpikeStarted();
}

void USpikeComponent::MulticastStopDefuseSpike_Implementation() {
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character) {
        return;
    }
    OnUpdateDefuseSpikeState.Broadcast(false);
    Character->OnDefuseSpikeStopped(); // Changed to OnDefuseSpikeStopped()
}

// callback from spike actor
void USpikeComponent::OnDefuseSucceed() {
    if (ActionStateComp) {
        ActionStateComp->TrySetState(EActionState::Idle);
    }
    UnlockMovement();
}