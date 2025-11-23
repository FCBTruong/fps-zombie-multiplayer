// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/ThrownProjectileStun.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Controllers/MyPlayerController.h"
#include "Characters/BaseCharacter.h"

void AThrownProjectileStun::OnExplode()
{
    UE_LOG(LogTemp, Log, TEXT("AThrownProjectileStun::ExplodeNow"));
    FVector ImpactPoint = GetActorLocation();
    MulticastExplode(ImpactPoint);
    SetLifeSpan(0.1f);
}

void AThrownProjectileStun::MulticastExplode_Implementation(const FVector& ImpactPoint)
{
    // Get the local player controller
    AMyPlayerController* PC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
    if (!PC) {
        return;
    }

    // Get the local pawn (player character)
    APawn* Pawn = PC->GetPawn();
    if (!Pawn) {
        return;
    }

    // Get camera (eyes) position of the local player
    FVector EyeLoc;
    FRotator EyeRot;
    PC->GetPlayerViewPoint(EyeLoc, EyeRot);

    // Distance check
    const float MaxFlashRange = 2000.f;
    float Dist = FVector::Dist(ImpactPoint, EyeLoc);
    if (Dist > MaxFlashRange)
    {
        return; 
    }

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(Pawn);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        ImpactPoint,
        EyeLoc,
        ECC_Visibility,
        Params
    );

    bool bCanSeeFlash = false;

    if (!bHit)
    {
        bCanSeeFlash = true;
    }
    else if (Hit.GetActor() == Pawn)
    {
        bCanSeeFlash = true;
    }

    if (!bCanSeeFlash)
    {
        return; 
    }

    // Compute flash strength (distance falloff 0–1)
    float Strength = 1.f - (Dist / MaxFlashRange);
    Strength = FMath::Clamp(Strength, 0.f, 1.f);

    if (Data && Data->ExplosionSFX)
    {
        UGameplayStatics::PlaySound2D(this, Data->ExplosionSFX);
    }

	// Get CharacterBase to apply flash effect
	ABaseCharacter* Character = Cast<ABaseCharacter>(Pawn);
    if (Character) {
		Character->PlayStunEffect(Strength);
    }

    // Apply flash on the local client (UI, audio, camera effect)
    //PC->ApplyFlash(Strength);
}
