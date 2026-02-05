// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InteractComponent.h"
#include "Components/PickupComponent.h"	
#include "Characters/BaseCharacter.h"

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
	TraceForPickup();
}

void UInteractComponent::TraceForPickup()
{
	FVector Start, End;
	FRotator Rot;
	GetOwner()->GetActorEyesViewPoint(Start, Rot);
	End = Start + Rot.Vector() * 300.f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	FColor LineColor = bHit ? FColor::Red : FColor::Green;
	//DrawDebugLine(GetWorld(), Start, End, LineColor, false, 0.1f, 0, 1.f);

	APickupItem* NewPickup = bHit ? Cast<APickupItem>(Hit.GetActor()) : nullptr;
	if (NewPickup != FocusedPickup)
	{
		FocusedPickup = NewPickup;
		
		ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
		if (FocusedPickup)
		{
			UE_LOG(LogTemp, Warning, TEXT("Focused on pickup: %s"), *FocusedPickup->GetItemName());
			ShowPickupMessage.Broadcast(FocusedPickup->GetItemName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No longer focused on any pickup"));
			HidePickupMessage.Broadcast();
		}
	}
}

// Deprecated
void UInteractComponent::TryPickup()
{
	UE_LOG(LogTemp, Warning, TEXT("pressed pickup"));
	if (FocusedPickup)
	{
		if (GetOwner()->HasAuthority())
		{
			HandlePickup_Internal(FocusedPickup);
		}
		else
		{
			ServerTryPickup(FocusedPickup);
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