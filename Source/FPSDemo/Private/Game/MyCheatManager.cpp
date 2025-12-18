// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/MyCheatManager.h"
#include "Characters/BaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/HealthComponent.h"
#include "Controllers/MyPlayerController.h"


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
        PC->PlayerUI->OnUpdateDefuseSpikeState(false);
        break;
    case 2:
        PC->PlayerUI->OnUpdateDefuseSpikeState(true);
        break;
    case 3:
        PC->PlayerUI->ShowNotiToast(FText::FromString("You got me!!"));
        break;
    }
}