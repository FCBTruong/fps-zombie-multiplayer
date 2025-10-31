// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/MyCheatManager.h"
#include "Characters/BaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/HealthComponent.h"


void UMyCheatManager::Live() {
	UE_LOG(LogTemp, Warning, TEXT("CheatManager: Live called"));
    APlayerController* PC = GetOuterAPlayerController();
    if (!PC) return;
    auto* Char = Cast<ABaseCharacter>(PC->GetPawn());
    if (!Char) return;
    Char->ServerRevive();
}