// Fill out your copyright notice in the Description page of Project Settings.


#include "Spike/Spike.h"
#include <Kismet/GameplayStatics.h>
#include "Components/AudioComponent.h"
// Sets default values
ASpike::ASpike()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASpike::BeginPlay()
{
	Super::BeginPlay();
	
    ExplodeSphere = Cast<UStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("ExplodeEffect")));

	if (ExplodeSphere)
	{
		ExplodeSphere->SetVisibility(false);
		ExplodeSphere->SetWorldScale3D(FVector::ZeroVector);
	}

    if (HasAuthority())
    {
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle_Explode,
            this,
            &ASpike::Explode,
            TimeExplode,      // delay
            false      // no loop
        );
    }

    if (ActiveSound)
    {
        ActiveSoundComp = UGameplayStatics::SpawnSoundAtLocation(
            this,
            ActiveSound,
            GetActorLocation()
        );
	}

    // play sound, effect planted success
    if (SpikePlantedSound)
    {
        UGameplayStatics::PlaySound2D(
            this,
            SpikePlantedSound
        );
	}
}

void ASpike::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsExploding && ExplodeSphere)
    {
        ExplodeTimer += DeltaTime;
        float Alpha = FMath::Clamp(ExplodeTimer / 0.5f, 0.f, 1.f);

        // scale animation: 0 - 3
        float Scale = FMath::Lerp(0.f, 8.f, Alpha);
        ExplodeSphere->SetWorldScale3D(FVector(Scale));

        if (Alpha >= 1.f)
        {
            bIsExploding = false; // stop anim
        }
    }
}

void ASpike::Explode()
{
    if (IsDefused)
    {
		return;
	}

    if (!HasAuthority()) {
        return;
    }

	Multicast_Explode();
}

void ASpike::Multicast_Explode_Implementation()
{
    if (!ExplodeSphere)
    {
        UE_LOG(LogTemp, Warning, TEXT("ExplodeSphere is null in ASpike::Explode"));
        return;
    }
    ExplodeSphere->SetVisibility(true);
    ExplodeSphere->SetWorldScale3D(FVector::ZeroVector);

    bIsExploding = true;
    ExplodeTimer = 0.f;

    if (ExplodeSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            ExplodeSound,
            GetActorLocation()
        );
    }
}

void ASpike::Defused()
{
    // stop explode timer
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Explode);
    // off sound

	IsDefused = true;

	Multicast_Defused();
    
    // destroy actor
    //Destroy();
}

void ASpike::Multicast_Defused_Implementation()
{
    if (SpikeDefuseedVoice)
    {
        UGameplayStatics::PlaySound2D(
            this,
            SpikeDefuseedVoice
        );
    }
    if (ActiveSoundComp)
    {
        ActiveSoundComp->Stop();
	}
}