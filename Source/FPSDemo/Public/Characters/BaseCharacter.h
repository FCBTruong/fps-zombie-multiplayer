// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/PickupComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/WeaponComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Weapons/WeaponTypes.h"
#include "Weapons/WeaponBase.h"
#include "Weapons/WeaponKnifeBasic.h"
#include "Net/UnrealNetwork.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/InteractComponent.h"
#include "UI/PlayerUI.h"
#include "Components/HealthComponent.h"
#include "BaseCharacter.generated.h"

UCLASS()
class FPSDEMO_API ABaseCharacter : public ACharacter
{
    GENERATED_BODY()

protected:
    UPickupComponent* PickupComponent;

    UInventoryComponent* InventoryComp;

    UInteractComponent* InteractComp;

    UWeaponComponent* WeaponComp;

	UHealthComponent* HealthComp;
public:
    ABaseCharacter();

    UPROPERTY(BlueprintReadWrite, Category = "State")
    bool bCloseToWall = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bReloading = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bEquipped = false;

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Crouching, Category = "State")
    bool bCrouching = false;

    UPROPERTY(BlueprintReadWrite, Category = "State")
    bool bIsFPS = false;

    UPROPERTY(ReplicatedUsing = OnRep_IsAiming, BlueprintReadOnly, Category = "State")
    bool bAiming = false;

    UPROPERTY()
	bool bHoldingShift = false;


    UPROPERTY(BlueprintReadOnly, Category = "Data")
    FVector2D moveInput;
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Data")
	FVector2D LookInput;
    UPROPERTY(BlueprintReadWrite, Category = "State")
	float AimSensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* FireMeleeMontage;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    TSubclassOf<AWeaponKnifeBasic> KnifeClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Input")
    UPlayerUI* PlayerUI;

    USplineComponent* ThrowSpline;
protected:
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRepSpeedWalkCurrently, Category = "Data")
	float SpeedWalkCurrently = NORMAL_WALK_SPEED;
    UFUNCTION()
	void OnRepSpeedWalkCurrently();

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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_CAMERA;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_SELECT_FIRST_RIFLE;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_SELECT_SECOND_RIFLE;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_SELECT_MELEE;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_SELECT_PISTOL;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_SELECT_THROWABLE;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_DROP_WEAPON;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_PICKUP;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_CHANGE_VIEW;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* CurrentCamera;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FirstPersonCamera;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* ThirdPersonCamera;

    USkeletalMeshComponent* mesh;
	USkeletalMeshComponent* MeshFps;

    UPROPERTY(EditDefaultsOnly, Category = "Crouch")
    UCurveFloat* CrouchCurve;   // assign in editor

    FTimeline CrouchTimeline;
    UFUNCTION()
    void HandleCrouchProgress(float Value);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    UAnimMontage* EquipMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    UAnimMontage* ThrowNadeMontage;

    // Lifecycle
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Input handlers
    void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
    void StartRunning();
    void StopRunning();
    virtual void Jump() override;
    virtual void StopJumping() override;
    void CustomCrouch();
    void CustomUnCrouch();
    void ClickCrouch();
	void ChangeView();
    UFUNCTION(BlueprintCallable)
	void UpdateView();
    UFUNCTION(BlueprintPure)
    EWeaponTypes GetWeaponType();
  
    UFUNCTION(Server, Reliable)
    void Server_UpdateLookInput(FVector2D NewLookInput);

    UFUNCTION(Server, Reliable)
    void ServerSetCrouching(bool bNewCrouching);

	UFUNCTION()
	void OnRep_Crouching();
    void UpdateAttachLocationWeapon();
    void DropWeapon();

    UFUNCTION(Server, Reliable)
    void ServerSetAiming(bool bNewAiming);

    UFUNCTION()
    void OnRep_IsAiming();

	virtual void UpdateAimingState();
    
public:
    virtual void Tick(float DeltaTime) override;
    static constexpr float MAX_WALK_SPEED = 600.f;
    static constexpr float NORMAL_WALK_SPEED = 400.f;
    static constexpr float MELEE_WALK_SPEED = 500.f;
    static constexpr float CROUCH_WALK_SPEED = 200.f;
    void AddWeapon(AWeaponBase* weapon);
    FORCEINLINE UPickupComponent* GetPickupComponent() const {
        return PickupComponent;
    }
    FORCEINLINE UInventoryComponent* GetInventoryComponent() const {
        return InventoryComp;
    }
    FORCEINLINE UWeaponComponent* GetWeaponComponent() const {
        return WeaponComp;
    }
    UFUNCTION()
    USkeletalMeshComponent* GetCurrentMesh();

    virtual void ClickAim();
    bool IsRunning();
	bool IsFpsViewMode() const { return bIsFPS; }
    void PlayEquipWeaponAnimation(EWeaponTypes WeaponType);
	float GetSpeedWalkCurrently();
    void SetSpeedWalkCurrently(float NewSpeed);
	void HandleUpdateSpeedWalkCurrently();
    virtual float TakeDamage(
        float DamageAmount,
        struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator,
        class AActor* DamageCauser
    ) override;

	void PlayThrowNadeMontage();
	bool IsCloseToWall() const { return bCloseToWall; }
};
