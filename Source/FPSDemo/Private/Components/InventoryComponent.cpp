// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameConstants.h"
#include "Items/ItemIds.h"
#include "Weapons/WeaponData.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	GMR = GetWorld()->GetGameInstance()->GetSubsystem<UGameManager>();
	Super::BeginPlay();
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			InitState();
		});

	// Default slot melee weapon
	if (GetOwnerRole() == ROLE_Authority)
	{
		
	}
}

void UInventoryComponent::InitState() {
	if (GetOwnerRole() == ROLE_Authority)
	{
		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryComponent: Client InitState called, triggering OnRep_Items"));
	}
}



// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}


