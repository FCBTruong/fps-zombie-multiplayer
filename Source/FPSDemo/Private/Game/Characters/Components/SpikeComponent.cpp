#include "Game/Characters/Components/SpikeComponent.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/Characters/Components/InventoryComponent.h"
#include "Game/Modes/Spike/SpikeMode.h"
#include "Game/Framework/ShooterGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Game/Subsystems/ActorManager.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "Kismet/KismetMathLibrary.h"
#include "Game/Characters/Components/ActionStateComponent.h"
#include "Game/Characters/Components/EquipComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Game/Modes/Spike/Spike.h"

USpikeComponent::USpikeComponent()
{
    SetIsReplicatedByDefault(true);
}

void USpikeComponent::BeginPlay()
{
    Super::BeginPlay();
    Character = Cast<ABaseCharacter>(GetOwner());
	check(Character);
   
    InventoryComp = Character->GetInventoryComponent();
    ActionStateComp = Character->GetActionStateComponent();
	EquipComp = Character->GetEquipComponent();

	check(InventoryComp);
	check(ActionStateComp);
	check(EquipComp);
}

void USpikeComponent::RequestPlantSpike()
{
    if (!IsEnabled()) {
        return;
    }
    if (!Character->IsAlive())
    {
        return;
    }

    if (!InventoryComp->HasSpike())
    {
        return;
    }

    if (!ActionStateComp->CanPlantNow())
    {
        return;
    }

    if (Character->IsLocallyControlled()) {
        if (!CanPlantHere()) {
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

    if (ActionStateComp->GetState() != EActionState::Planting) {
		UE_LOG(LogTemp, Warning, TEXT("USpikeComponent::RequestStopPlantSpike: Not currently planting"));
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
    if (!IsEnabled()) {
        return;
    }
    if (EquipComp->GetActiveItemId() != EItemId::SPIKE) {
		UE_LOG(LogTemp, Warning, TEXT("USpikeComponent::StartPlant_Internal: Spike is not active item"));
        return;
    }
    if (!ActionStateComp->CanPlantNow())
    {
		UE_LOG(LogTemp, Warning, TEXT("USpikeComponent::StartPlant_Internal: Cannot plant now due to action state"));
        return;
    }
    if (!CanPlantHere())
    {
        UE_LOG(LogTemp, Log, TEXT("USpikeComponent::StartPlant_Internal: Cannot plant here"));
        return;
    }

    bool Result = ActionStateComp->TrySetState(EActionState::Planting);
    if (!Result)
    {
		UE_LOG(LogTemp, Warning, TEXT("USpikeComponent::StartPlant_Internal: Failed to set state to Planting"));
        return;
    }
    LockMovement();
    MulticastStartPlantSpike();

    GetWorld()->GetTimerManager().ClearTimer(PlantTimerHandle);
    GetWorld()->GetTimerManager().SetTimer(
        PlantTimerHandle,
        this,
        &USpikeComponent::FinishPlantSpike,
        PlantTime,
        false
    );
}

void USpikeComponent::StopPlant_Internal()
{
    if (!IsEnabled()) {
        return;
    }
    if (!ActionStateComp->IsInState(EActionState::Planting)) {
		UE_LOG(LogTemp, Warning, TEXT("USpikeComponent::StopPlant_Internal: Not currently planting"));
        return;
	}

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
    if (!ActionStateComp->IsInState(EActionState::Planting)) {
		UE_LOG(LogTemp, Warning, TEXT("USpikeComponent::FinishPlantSpike: Not in planting state"));
        return;
    }
    if (!InventoryComp->HasSpike())
    {
		UE_LOG(LogTemp, Warning, TEXT("USpikeComponent::FinishPlantSpike: No spike in inventory"));
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
    EquipComp->AutoSelectBestWeapon();
}

bool USpikeComponent::CanPlantHere() const
{
    if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
    {
        if (!MoveComp->IsMovingOnGround())
        {
			UE_LOG(LogTemp, Log, TEXT("USpikeComponent::CanPlantHere: Character is not on the ground"));
            return false;
        }
    }
    AActorManager* ActorManager = AActorManager::Get(GetWorld());
    if (!ActorManager)
    {
        return false;
    }

    TArray<ATriggerBox*> BombAreas = {
        ActorManager->GetAreaBombA(),
        ActorManager->GetAreaBombB()
    };
    const FVector CharLocation = Character->GetActorLocation();
    for (ATriggerBox* Area : BombAreas)
    {
        if (!Area)
        {
            continue;
        }

        UBoxComponent* Box =
            Cast<UBoxComponent>(Area->GetCollisionComponent());
        if (!Box)
        {
            continue;
        }

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
    Character->OnPlantSpikeStarted();
	OnUpdatePlantSpikeState.Broadcast(true);
}

void USpikeComponent::MulticastStopPlantSpike_Implementation()
{
    Character->OnPlantSpikeStopped();
    OnUpdatePlantSpikeState.Broadcast(false);
}

void USpikeComponent::LockMovement()
{
    if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
    {
        bCachedJumpAllowed = MoveComp->IsJumpAllowed();
        MoveComp->DisableMovement();
        MoveComp->StopMovementImmediately();
        MoveComp->SetJumpAllowed(false);
    }
}

void USpikeComponent::UnlockMovement()
{
    if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
    {
        MoveComp->SetMovementMode(MOVE_Walking);
        MoveComp->SetJumpAllowed(bCachedJumpAllowed);
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
		UE_LOG(LogTemp, Warning, TEXT("ServerStartDefuseSpike: No GameState found"));
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

    ASpike* SpikeActor = GameState->GetPlantedSpike();
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
	if (Distance > DefuseDistance) { 
		UE_LOG(LogTemp, Warning, TEXT("ServerStartDefuseSpike: Too far from spike to defuse"));
		return;
	}

    if (!ActionStateComp->CanDefuseNow()) {
		UE_LOG(LogTemp, Warning, TEXT("ServerStartDefuseSpike: Cannot defuse now, current state: %d"), (int)ActionStateComp->GetState());
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
    if (!ActionStateComp->IsInState(EActionState::Defusing)) {
		UE_LOG(LogTemp, Warning, TEXT("StopDefuse_Internal: Not currently defusing"));
        return;
    }

    AShooterGameState* GS = Cast<AShooterGameState>(GetWorld()->GetGameState());
    ASpike* SpikeActor = GS->GetPlantedSpike();
    if (!SpikeActor) {
		UE_LOG(LogTemp, Warning, TEXT("StopDefuse_Internal: No planted spike found"));
        return;
    }
    if (!SpikeActor->IsDefuseInProgress()) {
		UE_LOG(LogTemp, Warning, TEXT("StopDefuse_Internal: No defuse in progress"));
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
    OnUpdateDefuseSpikeState.Broadcast(true);
    Character->OnDefuseSpikeStarted();
}

void USpikeComponent::MulticastStopDefuseSpike_Implementation() {
    OnUpdateDefuseSpikeState.Broadcast(false);
    Character->OnDefuseSpikeStopped(); // Changed to OnDefuseSpikeStopped()
}

// callback from spike actor
void USpikeComponent::OnDefuseSucceed() {
    ActionStateComp->TrySetState(EActionState::Idle);
    UnlockMovement();
}

void USpikeComponent::OnOwnerDead() {
    if (!IsEnabled()) {
        return;
    }
    if (ActionStateComp->IsInState(EActionState::Planting)) {
        StopPlant_Internal();
	}
    else if (ActionStateComp->IsInState(EActionState::Defusing)) {
        StopDefuse_Internal();
    }
}