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
    ConstructorHelpers::FObjectFinder<UMaterial> MaterialFinder(
        TEXT("/Game/Main/M_ViewmodelOverlay.M_ViewmodelOverlay")
    );

    if (MaterialFinder.Succeeded())
    {
        MaterialOverlayBase = MaterialFinder.Object;
    }
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
    bIsFPS = false;

    if (MeshFps) {
        MeshFps->bVisibleInSceneCaptureOnly = true;
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
    
    if (ViewmodelCap) {
        ViewmodelCap->FOVAngle = (DefaultFpsFov);
    }
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
    if (!CameraFps || !CameraTps || !ViewmodelCap || !MeshTps)
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

		MeshTps->SetVisibility(false);

        if (PC->IsLocalController())
        {
            ViewmodelCap->ShowOnlyComponents.Empty();
            if (MeshFps)
            {
                ViewmodelCap->ShowOnlyComponents.AddUnique(MeshFps);
            }
            ViewmodelCap->Activate();

			UE_LOG(LogTemp, Warning, TEXT("UCharCameraComponent::ApplyView - Updating viewmodel capture for FPS"));
            PC->UpdateViewmodelCapture(true);
        }
    }
    else
    {
        CameraFps->SetActive(false);
        CameraTps->SetActive(true);

        MeshTps->SetVisibility(true);

        if (PC->IsLocalController())
        {
            ViewmodelCap->ShowOnlyComponents.Empty();
            ViewmodelCap->Deactivate();
            PC->UpdateViewmodelCapture(false);      
        }
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

    // Build viewmodel RT + overlay only for local controller
    if (!ViewmodelRenderTarget)
    {
        ViewmodelRenderTarget = NewObject<UTextureRenderTarget2D>(this);
        ViewmodelRenderTarget->ClearColor = FLinearColor::Transparent;
		ViewmodelRenderTarget->InitAutoFormat(1920, 1080); // default size 16:9
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
        UE_LOG(LogTemp, Warning, TEXT("OverlayMID created and texture parameter set"));
    }

    if (AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC))
    {
		UE_LOG(LogTemp, Warning, TEXT("UCharCameraComponent::OnBecomeViewTarget - Setting viewmodel overlay")); 
        if (OverlayMID)
        {
            MyPC->NotifyViewmodelOverlayReady(OverlayMID);
        }
        else 
        {
            UE_LOG(LogTemp, Warning, TEXT("OverlayMID is null in OnBecomeViewTarget"));
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