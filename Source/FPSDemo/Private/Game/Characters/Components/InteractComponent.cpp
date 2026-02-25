// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Characters/Components/InteractComponent.h"
#include "Game/Characters/Components/PickupComponent.h"	
#include "Game/Characters/BaseCharacter.h"

// Sets default values for this component's properties
UInteractComponent::UInteractComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UInteractComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UInteractComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TraceFocus();
}

void UInteractComponent::TraceFocus()
{
	if (!GetOwner() || !GetWorld())
	{
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}
	if (!OwnerPawn->IsLocallyControlled()) {
		return;
	}

	constexpr float CharacterTraceRange = 500.0f;
	constexpr float PickupTraceRange = 300.0f;

	FVector Start;
	FRotator Rot;
	GetOwner()->GetActorEyesViewPoint(Start, Rot);

	const FVector End = Start + Rot.Vector() * CharacterTraceRange;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractTrace), false);
	Params.AddIgnoredActor(GetOwner());

	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	AActor* HitActor = bHit ? Hit.GetActor() : nullptr;

	// -------- Character focus --------
	ABaseCharacter* NewFocusedChar = Cast<ABaseCharacter>(HitActor);
	if (NewFocusedChar != FocusedChar.Get())
	{
		if (FocusedChar.Get())
		{
			FocusedChar->ShowNameText(false);
		}

		FocusedChar = NewFocusedChar;

		if (FocusedChar.Get())
		{
			FocusedChar->ShowNameText(true);
		}
	}

	// -------- Pickup focus --------
	APickupItem* NewFocusedPickup = nullptr;
	if (HitActor && Hit.Distance <= PickupTraceRange)
	{
		NewFocusedPickup = Cast<APickupItem>(HitActor);
	}

	if (!FocusedPickup.Get()) {
		HideInteractMessage.Broadcast();
	}

	if (NewFocusedPickup != FocusedPickup.Get())
	{
		FocusedPickup = NewFocusedPickup;

		if (NewFocusedPickup)	
		{
			ShowInteractMessage.Broadcast(NewFocusedPickup->GetItemName());
			UE_LOG(LogTemp, Warning, TEXT("Test 01"));
		}
	}
}

// Deprecated
void UInteractComponent::TryPickup()
{
	UE_LOG(LogTemp, Warning, TEXT("pressed pickup"));
	if (FocusedPickup.Get())
	{
		if (GetOwner()->HasAuthority())
		{
			HandlePickup_Internal(FocusedPickup.Get());
		}
		else
		{
			ServerTryPickup(FocusedPickup.Get());
		}
	}
}

void UInteractComponent::HandlePickup_Internal(APickupItem* Item)
{
	if (Item)
	{
		if (auto PickupComp = GetOwner()->FindComponentByClass<UPickupComponent>())
		{
			PickupComp->PickupItem(Item, true);
		}
	}
}

void UInteractComponent::ServerTryPickup_Implementation(APickupItem* Item)
{
	HandlePickup_Internal(Item);
}