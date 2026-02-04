// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CharCameraComponent.h"


#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Controllers/MyPlayerController.h"
#include "Kismet/GameplayStatics.h"


UCharCameraComponent::UCharCameraComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UCharCameraComponent::Initialize(
    UCameraComponent* InCameraFps,
    UCameraComponent* InCameraTps,
    USpringArmComponent* InCameraBoom,
    USkeletalMeshComponent* InMeshFps,
    USkeletalMeshComponent* InMeshTps
)
{
    CameraFps = InCameraFps;
    CameraTps = InCameraTps;
    CameraBoom = InCameraBoom;
    MeshFps = InMeshFps;
    MeshTps = InMeshTps;
    TargetFOV = DefaultFpsFov;
    bIsFPS = false;

    if (MeshFps) {
		MeshFps->SetOnlyOwnerSee(true);
        MeshFps->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
    }
    if (CameraFps) {
		CameraFps->bEnableFirstPersonFieldOfView = true;
		CameraFps->bEnableFirstPersonScale = true;
        CameraFps->SetFirstPersonScale(FIRST_PERSON_SCALE);
		CameraFps->SetFirstPersonFieldOfView(DefaultFpsFov);
    }
    if (CameraBoom) {
        CameraBoom->bUsePawnControlRotation = true; // camera rotates with mouse
        CameraBoom->bInheritYaw = true;
        CameraBoom->bInheritPitch = true;
        CameraBoom->bInheritRoll = false;
        CameraBoom->SetRelativeLocation(FVector(0, 30, 100));
		CameraBoom->TargetArmLength = 300.f;
    }
    if (CameraTps) {
        CameraTps->bUsePawnControlRotation = false; // camera does not rotate with mouse
	}   
	UE_LOG(LogTemp, Warning, TEXT("UCharCameraComponent::Initialize called"));
}

void UCharCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!CameraFps)
    {
        return;
    }

    const float CurrentFOV = CameraFps->FieldOfView;
    const float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, 10.f);
    CameraFps->SetFieldOfView(NewFOV);
	CameraFps->SetFirstPersonFieldOfView(NewFOV);
}

void UCharCameraComponent::BeginPlay()
{
    Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("UCharCameraComponent::BeginPlay called"));
}

bool UCharCameraComponent::IsFPS() const
{
    return bIsFPS;
}

void UCharCameraComponent::SetFPS(bool bNewIsFPS)
{
    if (bIsFPS == bNewIsFPS)
    {
        return;
    }

    bIsFPS = bNewIsFPS;
    ApplyView();
}

void UCharCameraComponent::ToggleView()
{
    bIsFPS = !bIsFPS;
    ApplyView();
}

void UCharCameraComponent::ApplyView()
{
    if (!CameraFps || !CameraTps || !MeshTps)
    {
        return;
    }

    if (bIsFPS)
    {
        CameraFps->SetActive(true);
        CameraTps->SetActive(false);

		MeshTps->SetOwnerNoSee(true);
		MeshFps->SetOwnerNoSee(false);
    }
    else
    {
        CameraFps->SetActive(false);
        CameraTps->SetActive(true);
		MeshFps->SetOwnerNoSee(true);

		MeshTps->SetOwnerNoSee(false);
    }

    OnViewModeChanged.Broadcast(bIsFPS);
}

void UCharCameraComponent::OnBecomeViewTarget(APlayerController* PC)
{
	UE_LOG(LogTemp, Warning, TEXT("UCharCameraComponent::OnBecomeViewTarget called"));
    if (!PC || !PC->IsLocalController())
    {
        return;
    }

    CachedLocalPC = PC;
    bIsFPS = false;
    TargetFOV = DefaultFpsFov;
    ApplyView();
}

void UCharCameraComponent::OnEndViewTarget(APlayerController* PC)
{
    if (PC && PC->IsLocalController())
    {
        bIsFPS = false;
        ApplyView();
    }

    if (PC && CachedLocalPC.Get() == PC)
    {
        CachedLocalPC = nullptr;
    }
}

void UCharCameraComponent::SetTargetFOV(float NewFOV)
{
    TargetFOV = NewFOV;
}

void UCharCameraComponent::ResetFOV()
{
    TargetFOV = DefaultFpsFov;
}

void UCharCameraComponent::SetAiming(bool bAiming)
{
    // Only apply FOV / UI-related visuals when the local player is viewing this pawn
    if (!IsLocalViewingOwner())
    {
        // Still update sensitivity so owner can query it consistently
        AimSensitivity = bAiming ? 0.4f : 1.0f;
        return;
    }

    if (bAiming)
    {
        SetTargetFOV(20.f);
        AimSensitivity = 0.4f;
        OnAimingVisualsChanged.Broadcast(true);
    }
    else
    {
        ResetFOV();
        AimSensitivity = 1.0f;
        OnAimingVisualsChanged.Broadcast(false);
    }
}

bool UCharCameraComponent::IsLocalViewingOwner() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return false;

    APlayerController* PC = CachedLocalPC.Get();
    if (!PC)
    {
        PC = UGameplayStatics::GetPlayerController(Owner, 0);
    }
    if (!PC || !PC->IsLocalController()) return false;

    return (PC->GetViewTarget() == Owner);
}