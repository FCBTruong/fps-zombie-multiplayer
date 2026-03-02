// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Characters/Components/InteractComponent.h"
#include "Game/Characters/Components/PickupComponent.h"	
#include "Game/Characters/BaseCharacter.h"
#include "Game/Modes/Spike/Spike.h"

// Sets default values for this component's properties
UInteractComponent::UInteractComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInteractComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ABaseCharacter>(GetOwner());
	check(OwnerCharacter);
}

void UInteractComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (OwnerCharacter->IsLocallyControlled()) {
		TraceFocus();
	}
}

void UInteractComponent::TraceFocus()
{
	if (!OwnerCharacter->IsLocallyControlled() || OwnerCharacter->IsDead())
	{
		return;
	}

	FVector Start;
	FRotator Rot;
	OwnerCharacter->GetActorEyesViewPoint(Start, Rot);

	const FVector End = Start + Rot.Vector() * FocusRange;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(FocusTrace), false);
	Params.AddIgnoredActor(OwnerCharacter);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	AActor* NewActor = bHit ? Hit.GetActor() : nullptr;

	SetFocus(NewActor);
}

void UInteractComponent::StartFocus(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	// Character focus behavior
	if (ABaseCharacter* Char = Cast<ABaseCharacter>(Actor))
	{
		Char->ShowNameText(true);
		return;
	}

	// Pickup focus behavior
	if (APickupItem* Pickup = Cast<APickupItem>(Actor))
	{
		ShowInteractMessage.Broadcast(
			FText::FromString(FString::Printf(TEXT("%s\n(Press F)"), *Pickup->GetItemName()))
		);
	}
	else if (Cast<ASpike>(Actor))
	{
		ShowInteractMessage.Broadcast(FText::FromString(TEXT("Press 'E' to defuse")));
	}
}

void UInteractComponent::EndFocus(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	if (ABaseCharacter* Char = Cast<ABaseCharacter>(Actor))
	{
		Char->ShowNameText(false);
	}
	else {
		HideInteractMessage.Broadcast();
	}
}

void UInteractComponent::SetFocus(AActor* NewActor)
{
	if (NewActor == FocusedActor.Get())
	{
		return;
	}

	// End old focus
	if (AActor* OldActor = FocusedActor.Get())
	{
		EndFocus(OldActor);
	}

	FocusedActor = NewActor;

	// Start new focus
	if (NewActor)
	{
		StartFocus(NewActor);
	}
}

void UInteractComponent::TryPickup()
{
	APickupItem* FocusedPickup = Cast<APickupItem>(FocusedActor.Get());
	if (!FocusedPickup)
	{
		return;
	}

	if (OwnerCharacter->HasAuthority())
	{
		HandlePickup_Internal(FocusedPickup);
	}
	else
	{
		ServerTryPickup(FocusedPickup);
	}
}

void UInteractComponent::HandlePickup_Internal(APickupItem* Item)
{
	if (!Item)
	{
		return;
	}

	UPickupComponent* PickupComp = OwnerCharacter->GetPickupComponent();
	check(PickupComp);
	PickupComp->PickupItem(Item, true);
}

void UInteractComponent::ServerTryPickup_Implementation(APickupItem* Item)
{
	HandlePickup_Internal(Item);
}