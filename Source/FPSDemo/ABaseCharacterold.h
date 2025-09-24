// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WeaponTypes.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "AWeaponBase.h"
#include "Components/TimelineComponent.h"
#include "ABaseCharacter.generated.h"


UCLASS()
class FPSDEMO_API AABaseCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AABaseCharacter();

    UPROPERTY(BlueprintReadOnly, Category = "Data")
	float Health = 100.f;
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bHoldingShoot = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bRunning = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bCloseToWall = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bReloading = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bEquipped = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bCrouching = false;


    UPROPERTY(BlueprintReadOnly, Category = "Data")
    FVector2D moveInput;
    UPROPERTY(BlueprintReadOnly, Category = "Data")
    EWeaponTypes weaponType = EWeaponTypes::Unarmed;

    // ABaseCharacter.h
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    TArray<AAWeaponBase*> Inventory;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    AAWeaponBase* CurrentWeapon;
protected:
    // Enhanced Input assets to assign in Editor
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_Movement;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_Shoot;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputMappingContext* IMC_FPS;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_JUMP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_AIM;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	class UInputAction* IA_RELOAD;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_RUN;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	class UInputAction* IA_CROUCH;
    UCameraComponent* cameraFps;
    USkeletalMeshComponent* mesh;

    UPROPERTY(EditDefaultsOnly, Category = "Crouch")
    UCurveFloat* CrouchCurve;   // assign in editor

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    TArray<AWeaponBase*> Inventory;

    FTimeline CrouchTimeline;
    UFUNCTION()
    void HandleCrouchProgress(float Value);

    void PlayCrouchTimeline(bool bCrouchDown);

    // Lifecycle
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Input handlers
    void StartFire();
    void FireRifle();
    bool CanShoot();
    void EquipWeapon();
    void ChangeViewMode();
    void Move(const FInputActionValue& Value);
	void StartRunning();
	void StopRunning();
    virtual void Jump() override;
    virtual void StopJumping() override;
	void CustomCrouch();
	void CustomUnCrouch();
	void ClickCrouch();

public:
    virtual void Tick(float DeltaTime) override;
    static constexpr float MAX_WALK_SPEED = 600.f;
	static constexpr float NORMAL_WALK_SPEED = 400.f;
	static constexpr float CROUCH_WALK_SPEED = 200.f;
};
