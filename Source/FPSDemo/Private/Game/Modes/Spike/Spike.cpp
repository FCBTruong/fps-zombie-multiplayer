// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Modes/Spike/Spike.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Game/Modes/Spike/SpikeMode.h"
#include "Game/Characters/Components/SpikeComponent.h"
#include "Game/Characters/BaseCharacter.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
ASpike::ASpike()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true; // carefully, BP_Spike override may turn this off
}

// Called when the game starts or when spawned
void ASpike::BeginPlay()
{
	Super::BeginPlay();
	
    ExplodeSphere = Cast<UStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("ExplodeEffect")));
    MainMeshRef = Cast<UStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("MainMesh")));

	if (ExplodeSphere)
	{
		ExplodeSphere->SetVisibility(false);
		ExplodeSphere->SetWorldScale3D(FVector::ZeroVector);
        ExplodeSphere->OnComponentBeginOverlap.AddDynamic(this, &ASpike::OnExplodeSphereBeginOverlap);
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

	SpringArmComp = FindComponentByClass<USpringArmComponent>();
    if (SpringArmComp) {
        CamPitch = SpringArmComp->GetRelativeRotation().Pitch;
    }
}

void ASpike::OnExplodeSphereBeginOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    if (!HasAuthority()) return;
    if (!IsValid(OtherActor) || OtherActor == this) return;

    ABaseCharacter* Character = Cast<ABaseCharacter>(OtherActor);
    if (!Character) return;
    if (!Character->IsAlive()) return;

    
    UGameplayStatics::ApplyDamage(
        Character,
        ExplodeDamage,          
        GetInstigatorController(),
        this,
        UDamageType::StaticClass()
    );
}

void ASpike::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (HasAuthority()) {
        if (bIsDefuseInProgress) {
            if (DefusingComponent.IsValid()) {
                // check if player is dead or alive
                ABaseCharacter* Character = Cast<ABaseCharacter>(DefusingComponent->GetOwner());
                if (Character && !Character->IsAlive()) {
                    CancelDefuse();
                    return;
                }
            }
        }
	}

    if (!ExplodeSphere) {
        return;
	}

    if (ExplosionState == ESpikeExplosionState::WaitingToShrink)
    {
		ExplodeTimer += DeltaTime;
        const float WaitDuration = 1.5f; // time to wait before shrinking
        if (ExplodeTimer >= WaitDuration)
        {
            ExplosionState = ESpikeExplosionState::Shrinking;
            ExplodeTimer = 0.f; // reset timer for shrinking
        }
	}

    if (ExplosionState == ESpikeExplosionState::Exploding)
    {
        ExplodeTimer += DeltaTime;
		const float Duration = 1.0f; // explosion anim duration in seconds
        float Alpha = FMath::Clamp(ExplodeTimer / Duration, 0.f, 1.f);

        // scale animation: 0 - 3
        float Scale = FMath::Lerp(0.f, ScaleRadius, Alpha);
        ExplodeSphere->SetWorldScale3D(FVector(Scale));

        if (Alpha >= 1.f)
        {
			ExplosionState = ESpikeExplosionState::WaitingToShrink;
			ExplodeTimer = 0.f; // reset timer for waiting
			OnCompleteExplode();
        }
    }

    if (ExplosionState == ESpikeExplosionState::Shrinking)
    {
        ExplodeTimer += DeltaTime;
        const float Duration = 0.5f; // shrinking anim duration in seconds
        float Alpha = FMath::Clamp(ExplodeTimer / Duration, 0.f, 1.f);
        // scale animation: 3 - 0
        float Scale = FMath::Lerp(ScaleRadius, 0.f, Alpha);
        ExplodeSphere->SetWorldScale3D(FVector(Scale));
        if (Alpha >= 1.f)
        {
            ExplosionState = ESpikeExplosionState::Inactive;
            ExplodeSphere->SetVisibility(false);
        }
    }
}

void ASpike::Explode()
{
    if (bIsDefused)
    {
		return;
	}
    if (bIsDefuseInProgress) {
        bIsDefuseInProgress = false;
    }

	// clear defuse timer if any
	GetWorld()->GetTimerManager().ClearTimer(DefuseTimerHandle);

	ASpikeMode* SpikeMode = Cast<ASpikeMode>(UGameplayStatics::GetGameMode(this));
    if (!SpikeMode) {
        UE_LOG(LogTemp, Warning, TEXT("Explode: No SpikeGM found"));
        return;
	}
	SpikeMode->OnSpikeExploded();

	Multicast_Explode();
}

void ASpike::Multicast_Explode_Implementation()
{
    if (!ExplodeSphere)
    {
        return;
    }
    ExplodeSphere->SetVisibility(true);
    ExplodeSphere->SetWorldScale3D(FVector::ZeroVector);

	ExplosionState = ESpikeExplosionState::Exploding;
    ExplodeTimer = 0.f;

    if (ExplodeSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            ExplodeSound,
            GetActorLocation()
        );
    }
    if (MainMeshRef) {
		MainMeshRef->SetVisibility(false, true);
    }
}

void ASpike::Defused()
{
    // stop explode timer
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Explode);

    bIsDefused = true;
	bIsDefuseInProgress = false;

    ASpikeMode* SpikeGM = Cast<ASpikeMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (!SpikeGM) {
        return;
    }
    if (!SpikeGM->IsSpikePlanted()) {
        return;
    }

    if (!DefusingComponent.IsValid()) {
        return;
    }
    DefusingComponent->OnDefuseSucceed();

    APawn* Pawn = Cast<APawn>(DefusingComponent->GetOwner());
    AController* PC = Cast<AController>(Pawn->GetController());
    if (!PC)
    {
        return;
    }
    SpikeGM->OnSpikeDefused(PC);

    Multicast_Defused();
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

    USceneComponent* EffectActiveRef = FindComponentByClass<USceneComponent>();

    if (EffectActiveRef)
    {
        EffectActiveRef->SetVisibility(false, true); // hide + propagate to children
    }
}

void ASpike::StartDefuse(USpikeComponent* DefuseComp)
{
    if (!DefuseComp || bIsDefuseInProgress || IsDefused())
    {
        return;
    }

    bIsDefuseInProgress = true;
    DefusingComponent = DefuseComp;

    GetWorld()->GetTimerManager().SetTimer(
        DefuseTimerHandle,
        this,
        &ASpike::Defused,
        DefuseTime, // time to defuse in seconds
        false // no loop
    );
}

bool ASpike::IsDefuseInProgress() const {
    return bIsDefuseInProgress;
}

bool ASpike::IsDefused() const {
    return bIsDefused;
}

void ASpike::CancelDefuse()
{
    bIsDefuseInProgress = false;
	GetWorld()->GetTimerManager().ClearTimer(DefuseTimerHandle);
	DefusingComponent.Reset();
}

void ASpike::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ActiveSoundComp)
    {
        ActiveSoundComp->Stop();
    }
    Super::EndPlay(EndPlayReason);
}

void ASpike::OnCompleteExplode() {
    if (!HasAuthority()) {
        return;
	}
}

void ASpike::AddCameraYaw(float Value)
{
    if (!SpringArmComp || FMath::IsNearlyZero(Value)) return;
    CamYaw += Value * LookSensitivity;
    SpringArmComp->SetRelativeRotation(FRotator(CamPitch, CamYaw, 0.f));
}

void ASpike::AddCameraPitch(float Value)
{
    if (!SpringArmComp || FMath::IsNearlyZero(Value)) return;
    CamPitch = FMath::Clamp(CamPitch + Value * LookSensitivity, MinPitch, MaxPitch);
    SpringArmComp->SetRelativeRotation(FRotator(CamPitch, CamYaw, 0.f));
}

void ASpike::OnCharacterDead(ABaseCharacter* DeadCharacter)
{
    if (!HasAuthority()) {
        return;
    }
    if (!DeadCharacter) {
        return;
    }
    APawn* Pawn = Cast<APawn>(DeadCharacter);
    if (!Pawn) {
        return;
    }
    
    // if defuser is dead, cancel defuse
    if (bIsDefuseInProgress && DefusingComponent.IsValid()) {
        APawn* DefusePawn = Cast<APawn>(DefusingComponent->GetOwner());
        if (DefusePawn && DefusePawn == Pawn) {
            CancelDefuse();
        }
    }
}