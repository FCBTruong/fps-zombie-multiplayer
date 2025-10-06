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
#include "BaseCharacter.generated.h"

UCLASS()
class FPSDEMO_API ABaseCharacter : public ACharacter
{
    GENERATED_BODY()

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = "true"))
    UPickupComponent* PickupComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = "true"))
    UInventoryComponent* InventoryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = "true"))
    UWeaponComponent* WeaponComponent;
public:
    ABaseCharacter();

    UPROPERTY(BlueprintReadOnly, Category = "Data")
    float Health = 100.f;
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bHoldingShoot = false;

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

    UPROPERTY(BlueprintReadWrite, Category = "State")
    bool bAiming = false;
	bool bHoldingShift = false;


    UPROPERTY(BlueprintReadOnly, Category = "Data")
    FVector2D moveInput;
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Data")
	FVector2D LookInput;
    UPROPERTY(BlueprintReadWrite, Category = "State")
	float AimSensitivity = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    TArray<AWeaponBase*> WeaponSlots;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    AWeaponBase* CurrentWeapon;
    FTimerHandle FireTimerHandle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* FireRifleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* FireMeleeMontage;

    UPROPERTY(EditAnywhere, Category = "Weapon")
    TSubclassOf<AWeaponKnifeBasic> KnifeClass;
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
    class UInputAction* IA_DROP_WEAPON;

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

    // Lifecycle
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Input handlers
    void StartFire();
	void StopFire();
    void FireRifle();
    bool CanShoot();
    void EquipWeapon();
    void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
    void StartRunning();
    void StopRunning();
    virtual void Jump() override;
    virtual void StopJumping() override;
    void CustomCrouch();
    void CustomUnCrouch();
    void ClickCrouch();
    void AddWeaponToSlot(AWeaponBase* NewWeapon, int32 SlotIndex);
	void EquipSlot(int32 SlotIndex);
	void ChangeView();
    UFUNCTION(BlueprintCallable)
	void UpdateView();
    UFUNCTION(BlueprintPure)
    EWeaponTypes GetWeaponType();
	bool IsRunning();
    USkeletalMeshComponent* GetCurrentMesh();

	// Server functions
    UFUNCTION(Server, Reliable)
    void ServerFire();
    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayFireRifle(FVector TargetPoint);
    UFUNCTION(Server, Reliable)
    void Server_UpdateLookInput(FVector2D NewLookInput);

    UFUNCTION(Server, Reliable)
    void ServerSetCrouching(bool bNewCrouching);

	UFUNCTION()
	void OnRep_Crouching();
    void UpdateAttachLocationWeapon();
    void DropWeapon();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayFireMelee();
    
public:
    virtual void Tick(float DeltaTime) override;
    static constexpr float MAX_WALK_SPEED = 600.f;
    static constexpr float NORMAL_WALK_SPEED = 400.f;
    static constexpr float CROUCH_WALK_SPEED = 200.f;
    static constexpr int SLOT_RIFLE_1 = 0;
    static constexpr int SLOT_RIFLE_2 = 1;
    static constexpr int SLOT_PISTOL = 2;
    static constexpr int SLOT_MELEE = 3;
    void AddWeapon(AWeaponBase* weapon);
    FORCEINLINE UPickupComponent* GetPickupComponent() const {
        return PickupComponent;
    }
};
