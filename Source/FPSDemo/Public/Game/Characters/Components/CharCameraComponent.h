// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharCameraComponent.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USkeletalMeshComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnViewModeChanged, bool /*bIsFPS*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimingVisualsChanged, bool /*bIsAiming*/);

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

    void SetAiming(bool bAiming);
    float GetAimSensitivity() const { return AimSensitivity; }

    FOnViewModeChanged OnViewModeChanged;
    FOnAimingVisualsChanged OnAimingVisualsChanged;
protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void BeginPlay() override;
protected:
    UPROPERTY(EditDefaultsOnly, Category = "Init|Camera")
    TSubclassOf<AActor> DeathCameraProxyClass;

private:
    UPROPERTY(Transient) TObjectPtr<UCameraComponent> CameraFps;
    UPROPERTY(Transient) TObjectPtr<UCameraComponent> CameraTps;
    UPROPERTY(Transient) TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(Transient) TObjectPtr<USkeletalMeshComponent> MeshFps;
    UPROPERTY(Transient) TObjectPtr<USkeletalMeshComponent> MeshTps;

    bool bIsFPS = true;
    float TargetFOV = 103.f;
    float DefaultFpsFov = 103.f;
    float AimSensitivity = 1.0f;
    TWeakObjectPtr<APlayerController> CachedLocalPC;
    void ApplyView();
    bool IsLocalViewingOwner() const;

	const float FIRST_PERSON_SCALE = 0.1f;
};
