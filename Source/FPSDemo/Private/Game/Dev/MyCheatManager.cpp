// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Dev/MyCheatManager.h"
#include "Game/Characters/BaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Game/Characters/Components/HealthComponent.h"
#include "Game/Framework/MyPlayerController.h"
#include "Game/UI/PlayerUI.h"
#include "Kismet/GameplayStatics.h"

void UMyCheatManager::Live() {
	UE_LOG(LogTemp, Warning, TEXT("CheatManager: Live called"));
    APlayerController* PC = GetOuterAPlayerController();
    if (!PC) return;
    auto* Char = Cast<ABaseCharacter>(PC->GetPawn());
    if (!Char) return;
}

void UMyCheatManager::Cmd(int Id) {
    UE_LOG(LogTemp, Warning, TEXT("CheatManager: Cmd called with Id: %d"), Id);
    AMyPlayerController* PC = Cast<AMyPlayerController>(GetOuterAPlayerController());
    if (!PC) return;
    
    switch (Id) {
    case 1:
        PC->GetPlayerUI()->OnUpdateDefuseSpikeState(false);
        break;
    case 2:
        PC->GetPlayerUI()->OnUpdateDefuseSpikeState(true);
        break;
    case 3:
        PC->GetPlayerUI()->ShowNotiToast(FText::FromString("You got me!!"));
        break;
    case 4:
        PC->Test();
        break;
    case 5:
		UE_LOG(LogTemp, Warning, TEXT("CheatManager: Cmd 5 - Loading Lobby Level"));
        /*UGameplayStatics::OpenLevel(this, FGameConstants::LEVEL_LOBBY);*/
        PC->ClientTravel(FGameConstants::LEVEL_LOBBY.ToString(), ETravelType::TRAVEL_Absolute);
        break;
    }
}


void UMyCheatManager::BecomeDC()
{
    UE_LOG(LogTemp, Warning, TEXT("CheatManager: BecomeDC called"));
    UWorld* World = GetWorld();
    if (!World) return;

    // Loads map named "Entry" (must be in Project Settings -> Packaging -> Maps to Cook, or referenced)
    UGameplayStatics::OpenLevel(World, FName(TEXT("/Game/Main/Levels/L_DedicatedServer")));
}
