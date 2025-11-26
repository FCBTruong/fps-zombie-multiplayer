// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/MyPlayerState.h"
#include "Characters/BaseCharacter.h"


AMyPlayerState::AMyPlayerState()
{
}

void AMyPlayerState::ProcessBuy(const UItemData* Item)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProcessBuy called with null Item"));
		return;
	}
	// Implement your buy logic here
	UE_LOG(LogTemp, Warning, TEXT("Processing buy for item: %s"), *Item->GetName());

	if (Money >= Item->Price)
	{
		// check whether the player can carry the item, etc.
		
		// add item to inventory logic would go here
		Money -= Item->Price;

		ABaseCharacter* MyChar = Cast<ABaseCharacter>(GetPawn());
		if (MyChar)
		{
			MyChar->GetInventoryComponent()->AddItem(Item);
		}


		UE_LOG(LogTemp, Warning, TEXT("Item purchased! Remaining money: %d"), Money);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough money to buy item: %s"), *Item->GetName());
	}
}