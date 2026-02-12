// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Modes/Practice/PracticeGameMode.h"
#include "Game/UI/PlayerUI.h"
#include "Game/Framework/MyPlayerController.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "Shared/Data/Items/FirearmConfig.h"
#include "Game/Subsystems/ItemsManager.h"

void APracticeGameMode::BeginPlay()
{
	Super::BeginPlay();
	UGameManager* GMR = UGameManager::Get(GetWorld());
	FVector CenterPos(1100.269357, 3047.719772, 50);
	// gen weapons
	// Weapon list
	TArray<EItemId> Items = {
		EItemId::RIFLE_AK_47,
		EItemId::RIFLE_RUSSIAN_AS_VAL,
		EItemId::RIFLE_M16A,
		EItemId::RIFLE_QBZ,
		EItemId::SNIPER_BOLT_R,
		EItemId::GRENADE_FRAG_BASIC,
		EItemId::GRENADE_INCENDIARY,
		EItemId::GRENADE_SMOKE,
		EItemId::GRENADE_STUN
	};

	const float Distance = 120.0f;
	
	// Center the row
	int32 Count = Items.Num();
	float StartOffset = -((Count - 1) * Distance) * 0.5f;

	auto ItemsManager = UItemsManager::Get(GetWorld());
	for (int32 i = 0; i < Count; ++i)
	{
		FPickupData P;
		P.Id = GMR->GetNextItemOnMapId();
		P.ItemId = Items[i];
		P.Amount = 1;

		const UItemConfig* ItemConfig = ItemsManager->GetItemById(Items[i]);
		const UFirearmConfig* FirearmConfig = Cast<UFirearmConfig>(ItemConfig);
		if (FirearmConfig)
		{
			P.AmmoInClip = FirearmConfig->MaxAmmoInClip;
			P.AmmoReserve = FirearmConfig->MaxAmmoInClip * 10;
		}


		// Offset along X axis
		P.Location = CenterPos + FVector(StartOffset + i * Distance, 0.f, 0.f);

		APickupItem* PickupActor = GMR->CreatePickupActor(P);
		UE_LOG(LogTemp, Warning, TEXT("Pickup %d address = %p"), i, PickupActor);
	}
}