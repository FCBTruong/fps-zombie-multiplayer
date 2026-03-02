// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "Shared/Types/ItemId.h"
#include "Shared/Types/EquippedAnimState.h"
#include "Game/Characters/CharacterRole.h"
#include "Game/Types/TeamId.h"
#include "GameConstants.h"
#include "BaseCharacter.generated.h"

class UPickupComponent;
class UInventoryComponent;
class UInteractComponent;
class UHealthComponent;
class UCameraComponent;
class USpikeComponent;
class UEquipComponent;
class USpringArmComponent;
class USceneCaptureComponent2D;
class USkeletalMeshComponent;
class USceneComponent;
class UCurveFloat;
class UAnimMontage;
class UNiagaraSystem;
class UAIPerceptionStimuliSourceComponent;
class USoundBase;
class UMaterial;
class UMaterialInstanceDynamic;
class UMaterialParameterCollection;
class UTextureRenderTarget2D;
class UAudioComponent;
class USplineComponent;
class UAnimationComponent;
class UCharAudioComponent;
class UCharCameraComponent;
class UActionStateComponent;
class UItemVisualComponent;
class UWeaponFireComponent;
class UWeaponMeleeComponent;
class UThrowableComponent;
class UCharacterAsset;
class UPostProcessComponent;
class UItemConfig;
class URoleComponent;
class UItemUseComponent;
class UTextRenderComponent;
class ULagCompensationComponent;

DECLARE_MULTICAST_DELEGATE(FOnHit);

UENUM(BlueprintType)
enum class EMovementState : uint8
{
    Normal,
	Slow
};

UENUM()
enum class EAimPointPolicy : uint8
{
    Center,
    Head,
    Chest,
    Pelvis,
    RandomBody,
    HeadOrBody
};

UCLASS()
class FPSDEMO_API ABaseCharacter : public ACharacter
{
    GENERATED_BODY()

public:
	ABaseCharacter();
    virtual void GetActorEyesViewPoint(FVector& out_Location, FRotator& out_Rotation) const override;
protected:
    //  ===== Lifecycle =====
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void Landed(const FHitResult& Hit) override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(
        float DamageAmount,
        struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator,
        class AActor* DamageCauser
    ) override;
    virtual void Jump() override;
    virtual void StopJumping() override;
    virtual void Destroyed() override;
    virtual void BecomeViewTarget(APlayerController* PC) override;
    virtual void EndViewTarget(APlayerController* PC) override;
    virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
    virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
protected:
	// Components

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UPickupComponent> PickupComponent;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UInventoryComponent> InventoryComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UInteractComponent> InteractComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UHealthComponent> HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UAnimationComponent> AnimationComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UCharAudioComponent> AudioComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCharCameraComponent> CameraComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UEquipComponent> EquipComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USpikeComponent> SpikeComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UItemVisualComponent> ItemVisualComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UActionStateComponent> ActionStateComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UWeaponFireComponent> WeaponFireComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UWeaponMeleeComponent> WeaponMeleeComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<ULagCompensationComponent> LagCompensationComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<URoleComponent> RoleComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UItemUseComponent> ItemUseComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UThrowableComponent> ThrowableComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> FpsPivot;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    TObjectPtr<UCameraComponent> CameraFps;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> MeshFps;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UCameraComponent> CameraTps;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UAIPerceptionStimuliSourceComponent> StimuliSource;

	// ===== Init Data =====
    UPROPERTY(Transient)
    TObjectPtr<UCharacterAsset> CachedCharacterAsset;
protected:
    // ===== Runtime State =====
    bool bLastHitWasHeadshot;
    float LastFootstepTime;
    float BaseStunDuration;
    float BasePivotFpsZ = 0.f;
    float CurrentCrouchCompZ = 0.f;
    float CrouchFromZ = 0.f;
    float CrouchToZ = 0.f;

    UPROPERTY()
    TObjectPtr<UAudioComponent> DefuseSpikeAudioComp;

    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> PlantSpikeAudioComp;

    UPROPERTY()
    TWeakObjectPtr<AController> LastHitByController;

    UPROPERTY()
    TArray<TWeakObjectPtr<AController>> DamageInstigators;

    UPROPERTY()
    TWeakObjectPtr<const UItemConfig> LastDamageCauser;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> FlashMID;

    UPROPERTY(Transient)
    TWeakObjectPtr<AActor> DeathCameraProxy;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* NameText;
protected:
	// ===== Replicated Properties =====
    UPROPERTY(ReplicatedUsing = OnRep_IsAiming)
    bool bIsAiming = false;

    UPROPERTY(ReplicatedUsing = OnRep_IsPermanentDead)
    bool bIsPermanentDead = false;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentMovementState)
    EMovementState CurrentMovementState = EMovementState::Normal;

    UPROPERTY(ReplicatedUsing = OnRep_CharacterSkin)
    int32 CharacterSkin = FGameConstants::SKIN_CHARACTER_ATTACKER;

    UPROPERTY(ReplicatedUsing = OnRep_SpeedMultiplier)
    float SpeedMultiplier = 1.0f;

    float SpineKickAlpha = 0.f;
    float FootstepInterval = 0.3f;
protected:
    // ===== Timelines =====
	FTimeline StunTimeline;
    FTimeline CrouchTimeline;
    FTimeline SpineKickTimeline;
    FTimerHandle HitSlowTimer;
	FTimerHandle SpectatorViewTimer;
protected:
    // ===== Internal Functions =====
    void OnMeleeNotify();
    void PlayFootstepSound();
    void UpdateFootstepSound(float DeltaTime);
    void PlayLandingSound();
    void OnHealthDepleted();
    void UpdateCurrentWeapon(EItemId CurrentWeaponId);
    void HandleRoleChanged(ECharacterRole OldRole, ECharacterRole NewRole);
	void ApplyDefaultsForRole(ECharacterRole NewRole);
    void ApplyVisualByRole(ECharacterRole NewRole);
	void ApplyLoadoutByRole(ECharacterRole NewRole);
    void BindMontageNotifies();
    void ApplyHitSlow(float Multiplier, float Duration);
    void ClearHitSlow();
    void BindDelegates();
    void SetupCrouchTimeline();
    void SetupStunTimeline();
    void SetupPerception();
    void SetupFlashPostProcess();
    void SetupInitialInventory();
    void BecomeHero_Internal();
    void PlayZombieSpawnEffects();
	void UpdateNameTextRotation();
    void DisableDeadMeshTick();
    bool CanPlayFootstep() const;
    bool IsBot() const;
    bool IsSpikeMode() const;
    FVector GetAimPointInternal(EAimPointPolicy Policy, float HeadChance01) const;

    UFUNCTION(BlueprintPure)
    EEquippedAnimState GetEquippedAnimState() const;

    UFUNCTION()
    void HandleCrouchProgress(float Alpha);

    // ===== Networking RPC =====
    UFUNCTION(Server, Unreliable)
    void ServerSetAiming(bool bNewAiming);
    UFUNCTION(Server, Reliable)
    void ServerSetIsSlow(bool bNewIsSlow); 

    // ===== Networking Multicast =====
    UFUNCTION(NetMulticast, Reliable)
    void MulticastCharacterDeath();
    UFUNCTION(NetMulticast, Reliable)
    void MulticastRevive();

	// ===== Networking OnRep =====
    UFUNCTION()
    void OnRep_IsAiming();
    UFUNCTION()
    void OnRep_CurrentMovementState();
    UFUNCTION()
    void OnRep_SpeedMultiplier();
	UFUNCTION()
	void OnRep_CharacterSkin();
    UFUNCTION()
	void OnRep_IsPermanentDead();

	// ===== Client RPC =====
    UFUNCTION(Client, Reliable)
    void ClientPlayHitEffect();

	// ===== Other UFUNCTIONS =====
    UFUNCTION()
    void OnStunTimelineFinished();
    UFUNCTION()
    void OnStunTimelineUpdate(float Value);
    UFUNCTION()
    void OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);
  
public:
    // ===== Public API =====
    void RequestStartAiming();
    void RequestStopAiming();
    void UpdateMaxWalkSpeed();
    void PlayBloodFx(const FVector& HitLocation, const FVector& HitNormal);
    void PlayStunEffect(const float& Strength);
    void ChangeView();
    void OnPlantSpikeStarted();
    void OnPlantSpikeStopped();
    void OnDefuseSpikeStarted();
    void OnDefuseSpikeStopped();
    void RequestCrouch();
    void RequestUnCrouch();
    void RequestSlowMovement(bool bEnable);
    void RequestJump();
    void RequestBecomeHero();
    void RequestPrimaryActionPressed();
    void RequestPrimaryActionReleased();
    void RequestSecondaryActionPressed();
    void RequestSecondaryActionReleased();
    void RequestReloadPressed();
	void Revive();
    void ApplyRealDeath(bool bDropInventory, bool bInPermanentDead = true);
    void PlayEffectHitReact();
    void SetCharacterSkin(int32 SkinId);
    void SetupSpineKickTimeline();
	void ShowNameText(bool bShow);
	bool Heal(float HealAmount);
    float GetAimSensitivity() const;
    bool IsHero() const;
    bool IsZombie() const;
    bool IsPermanentDead() const;
    bool CanAct();
    bool CanSeeThisActor(const APawn* Target) const;
    bool IsAlive() const;
    bool IsDead() const;
    bool IsFpsViewMode() const;
    bool IsAiming() const;
    bool IsCharacterRole(ECharacterRole InRole) const;
    ETeamId GetTeamId() const;
    FVector GetAimPoint(EAimPointPolicy Policy = EAimPointPolicy::Head, float HeadChance01 = 0.35f) const;
    FVector GetThrowableLocation() const;
    FString GetPlayerName() const;
    ECharacterRole GetCharacterRole() const;

	// ===== Getters =====
    UPickupComponent* GetPickupComponent() const;
    UInventoryComponent* GetInventoryComponent() const;
	UHealthComponent* GetHealthComponent() const;
	UInteractComponent* GetInteractComponent() const;
	UAnimationComponent* GetAnimationComponent() const;
    USkeletalMeshComponent* GetCurrentMesh() const;
	USkeletalMeshComponent* GetMeshFps() const;
	UEquipComponent* GetEquipComponent() const;
	UWeaponFireComponent* GetWeaponFireComponent() const;
	UWeaponMeleeComponent* GetWeaponMeleeComponent() const;
	UThrowableComponent* GetThrowableComponent() const; 
	USpikeComponent* GetSpikeComponent() const;
	UActionStateComponent* GetActionStateComponent() const;
    UItemVisualComponent* GetItemVisualComponent() const;
	UCharAudioComponent* GetAudioComponent() const;
	URoleComponent* GetRoleComponent() const;
	UCharCameraComponent* GetCharCameraComponent() const;
	ULagCompensationComponent* GetLagCompensationComponent() const;

    UFUNCTION(BlueprintCallable)
    EMovementState GetCurrentMovementState() const;

    UFUNCTION(BlueprintCallable)
    bool IsWorkingWithSpike() const;

    UFUNCTION(BlueprintCallable)
    float GetSpineKickAlpha() const;

    UFUNCTION(Server, Reliable)
    void ServerBecomeHero();

    UFUNCTION()
    void OnSpineKickUpdate(float Value);

	// ===== Delegates =====
    FOnHit OnHit;

	// ===== Constants =====
    static constexpr float MAX_WALK_SPEED = 600.f;
    static constexpr float NORMAL_WALK_SPEED = 400.f;
    static constexpr float CROUCH_WALK_SPEED = 200.f;
    static constexpr float SLOW_WALK_SPEED = 180.f;
    static constexpr float FOOTSTEP_SPEED_MIN = 300.f;
    static const FVector TPSMeshRelLoc;
    static const FRotator TPSMeshRelRot;
};
