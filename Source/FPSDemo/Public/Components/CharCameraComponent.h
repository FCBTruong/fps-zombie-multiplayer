// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharCameraComponent.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USceneCaptureComponent2D;
class USkeletalMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UCharCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UCharCameraComponent();
    void Initialize(
        UCameraComponent* InCameraFps,
        UCameraComponent* InCameraTps,
        USpringArmComponent* InCameraBoom,
        USceneCaptureComponent2D* InViewmodelCap,
        USkeletalMeshComponent* InMeshFps,
        USkeletalMeshComponent* InMeshTps
    );

    void SetFPS(bool bEnable);
    void ToggleView();
    bool IsFPS() const;
	void SetTargetFOV(float NewFOV);
	void ResetFOV();
	
    void OnBecomeViewTarget(APlayerController* PC);
    void OnEndViewTarget(APlayerController* PC);

    USceneCaptureComponent2D* GetViewmodelCapture() const;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Init|Viewmodel")
    TObjectPtr<UMaterial> MaterialOverlayBase;

    UPROPERTY(EditDefaultsOnly, Category = "Init|Camera")
    TSubclassOf<AActor> DeathCameraProxyClass;

private:
    UPROPERTY(Transient) TObjectPtr<UCameraComponent> CameraFps;
    UPROPERTY(Transient) TObjectPtr<UCameraComponent> CameraTps;
    UPROPERTY(Transient) TObjectPtr<USpringArmComponent> CameraBoom;
    UPROPERTY(Transient) TObjectPtr<USceneCaptureComponent2D> ViewmodelCap;

    UPROPERTY(Transient) TObjectPtr<USkeletalMeshComponent> MeshFps;
    UPROPERTY(Transient) TObjectPtr<USkeletalMeshComponent> MeshTps;

    UPROPERTY(Transient) TObjectPtr<UTextureRenderTarget2D> ViewmodelRenderTarget;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> OverlayMID;

    bool bIsFPS = true;
    float TargetFOV = 103.f;
    float DefaultFpsFov = 103.f;

    void ApplyView();
};
