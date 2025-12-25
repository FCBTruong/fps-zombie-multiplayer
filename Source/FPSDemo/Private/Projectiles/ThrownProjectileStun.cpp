// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/ThrownProjectileStun.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Controllers/MyPlayerController.h"
#include "Characters/BaseCharacter.h"
#include "Items/ThrowableConfig.h"

void AThrownProjectileStun::OnExplode()
{
    UE_LOG(LogTemp, Log, TEXT("AThrownProjectileStun::ExplodeNow"));
    FVector ImpactPoint = GetActorLocation();
    MulticastExplode(ImpactPoint);
    SetLifeSpan(0.1f);
}

void AThrownProjectileStun::MulticastExplode_Implementation(const FVector& ImpactPoint)
{
    // FX / SFX for everyone
    if (Data && Data->SmokeFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            Data->SmokeFX,
            ImpactPoint
        );
    }

    if (Data && Data->ExplosionSFX)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Data->ExplosionSFX, ImpactPoint);
    }

    // Local-only flash effect check (per client)
    AMyPlayerController* PC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
    if (!PC) return;

    APawn* Pawn = PC->GetPawn();
    if (!Pawn) return;

    // Eye/camera position + view rotation
    FVector EyeLoc;
    FRotator EyeRot;
    PC->GetPlayerViewPoint(EyeLoc, EyeRot);

    // Distance check
    constexpr float MaxFlashRange = 2000.f;
    const float Dist = FVector::Dist(ImpactPoint, EyeLoc);
    if (Dist > MaxFlashRange)
    {
        return;
    }

    // Line of sight check: from flash -> eye
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(FlashTrace), true);
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(Pawn);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        ImpactPoint,
        EyeLoc,
        ECC_Visibility,
        Params
    ); 

    bool bCanSeeFlash = false;
    if (!bHit)
    {
        bCanSeeFlash = true; // nothing blocks it
    }
    else if (Hit.GetActor() == Pawn)
    {
        bCanSeeFlash = true; // directly hit the pawn
    }

    if (!bCanSeeFlash)
    {
		UE_LOG(LogTemp, Log, TEXT("Flash blocked by %s"), *Hit.GetActor()->GetName());
        return;
    }

    // Look direction check (must be looking at flash within cone)
    const FVector ViewDir = EyeRot.Vector().GetSafeNormal();
    const FVector ToFlashDir = (ImpactPoint - EyeLoc).GetSafeNormal();

    // 0.7 = 45 degrees cone. Adjust as needed.
    constexpr float LookThreshold = 0.7f;

    const float LookDot = FVector::DotProduct(ViewDir, ToFlashDir);
	bool bIsLookingAtFlash = (LookDot >= LookThreshold);

     // Strength: distance falloff * angle factor
    float Strength = 1.f - (Dist / MaxFlashRange);
    Strength = FMath::Clamp(Strength, 0.f, 1.f);


    if (!bIsLookingAtFlash)
    {
		Strength *= 0.3f; // reduce strength if not directly looking
	}
    else {
        const float AngleFactor = FMath::Clamp(
            (LookDot - LookThreshold) / (1.f - LookThreshold),
            0.f,
            1.f
        );

        Strength *= AngleFactor;
    }
    
    if (Strength <= 0.f)
    {
        return;
    }

    // Local affect sound
    if (Data && Data->AffectSFX)
    {
        UGameplayStatics::PlaySound2D(this, Data->AffectSFX);
    }

    // Apply stun/flash effect on local player's character
    if (ABaseCharacter* Character = Cast<ABaseCharacter>(Pawn))
    {
        Character->PlayStunEffect(Strength);
    }
}
