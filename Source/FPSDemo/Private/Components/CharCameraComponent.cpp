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
    PrimaryComponentTick.bCanEverTick = true;
}

void UCharCameraComponent::Initialize(
    UCameraComponent* InCameraFps,
    UCameraComponent* InCameraTps,
    USpringArmComponent* InCameraBoom,
    USceneCaptureComponent2D* InViewmodelCap,
    USkeletalMeshComponent* InMeshFps,
    USkeletalMeshComponent* InMeshTps
)
{
    CameraFps = InCameraFps;
    CameraTps = InCameraTps;
    CameraBoom = InCameraBoom;
    ViewmodelCap = InViewmodelCap;
    MeshFps = InMeshFps;
    MeshTps = InMeshTps;
    TargetFOV = DefaultFpsFov;
    bIsFPS = true;
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
    if (!CameraFps || !CameraTps || !ViewmodelCap || !MeshTps)
    {
        return;
    }

    if (bIsFPS)
    {
        CameraFps->SetActive(true);
        CameraTps->SetActive(false);

        MeshTps->SetOwnerNoSee(true);

        ViewmodelCap->ShowOnlyComponents.Empty();
        if (MeshFps)
        {
            ViewmodelCap->ShowOnlyComponents.AddUnique(MeshFps);
        }
        ViewmodelCap->Activate();

        if (APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController()))
        {
            if (AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC))
            {
                if (MyPC->IsLocalController())
                {
                    MyPC->UpdateViewmodelCapture(true);
                }
            }
        }
    }
    else
    {
        CameraFps->SetActive(false);
        CameraTps->SetActive(true);

        MeshTps->SetOwnerNoSee(false);

        ViewmodelCap->ShowOnlyComponents.Empty();
        ViewmodelCap->Deactivate();

        if (APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController()))
        {
            if (AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC))
            {
                if (MyPC->IsLocalController())
                {
                    MyPC->UpdateViewmodelCapture(false);
                }
            }
        }
    }
}

void UCharCameraComponent::OnBecomeViewTarget(APlayerController* PC)
{
	UE_LOG(LogTemp, Warning, TEXT("UCharCameraComponent::OnBecomeViewTarget called"));
    if (!PC || !PC->IsLocalController())
    {
        return;
    }

    // Build viewmodel RT + overlay only for local controller
    if (!ViewmodelRenderTarget)
    {
        ViewmodelRenderTarget = NewObject<UTextureRenderTarget2D>(this);
        ViewmodelRenderTarget->ClearColor = FLinearColor::Transparent;

        int32 SizeX = 0, SizeY = 0;
        PC->GetViewportSize(SizeX, SizeY);
        SizeX = FMath::Max(SizeX, 1);
        SizeY = FMath::Max(SizeY, 1);

        ViewmodelRenderTarget->InitAutoFormat(SizeX, SizeY);
    }

    if (ViewmodelCap)
    {
        ViewmodelCap->TextureTarget = ViewmodelRenderTarget;
        ViewmodelCap->Activate();
    }

    if (MaterialOverlayBase && !OverlayMID)
    {
        OverlayMID = UMaterialInstanceDynamic::Create(MaterialOverlayBase, this);
        OverlayMID->SetTextureParameterValue(TEXT("ViewmodelTexture"), ViewmodelRenderTarget);
    }

    if (AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC))
    {
        if (OverlayMID)
        {
            MyPC->SetViewmodelOverlay(OverlayMID);
            MyPC->UpdateViewmodelCapture(true);
        }
    }

    bIsFPS = true;
    TargetFOV = DefaultFpsFov;
    ApplyView();
}

void UCharCameraComponent::OnEndViewTarget(APlayerController* PC)
{
    if (ViewmodelCap)
    {
        ViewmodelCap->ShowOnlyComponents.Empty();
        ViewmodelCap->Deactivate();
    }

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