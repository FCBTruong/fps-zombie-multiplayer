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


UCharCameraComponent::UCharCameraComponent()
{
    
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
        CameraFps->SetFirstPersonScale(0.2f);
		CameraFps->SetFirstPersonFieldOfView(DefaultFpsFov);
    }
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
}

void UCharCameraComponent::BeginPlay()
{
    Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("UCharCameraComponent::BeginPlay called"));
  
	CameraBoom->bInheritYaw = false;
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
    APawn* Pawn = Cast<APawn>(GetOwner());
    if (!Pawn)
    {
        return;
    }

    AMyPlayerController* PC = Cast<AMyPlayerController>(Pawn->GetController());
    if (!PC)
    {
        return;
    }

    if (bIsFPS)
    {
        CameraFps->SetActive(true);
        CameraTps->SetActive(false);

		MeshTps->SetOwnerNoSee(true);
    }
    else
    {
        CameraFps->SetActive(false);
        CameraTps->SetActive(true);

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

    bIsFPS = true;
    TargetFOV = DefaultFpsFov;
    ApplyView();
}

void UCharCameraComponent::OnEndViewTarget(APlayerController* PC)
{
    // Optional: when local controller stops viewing this pawn, force TPS
    if (PC && PC->IsLocalController())
    {
        bIsFPS = false;
        ApplyView();
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