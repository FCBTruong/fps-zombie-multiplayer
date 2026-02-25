#include "Game/Characters/BaseCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Game/Framework/ShooterGameMode.h"
#include "Game/Projectiles/ThrownProjectile.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Game/Framework/MyPlayerController.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Game/Utils/Damage/MyDamageType.h"
#include "Engine/DamageEvents.h"
#include "Game/Utils/Damage/MyPointDamageEvent.h"
#include "Game/Framework/MyPlayerState.h"
#include "Game/Framework/ShooterGameState.h"
#include "Perception/AISense_Damage.h"
#include "Game/AI/BotAIController.h"
#include "Game/Items/Weapons/WeaponState.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "Components/TimelineComponent.h"
#include "Game/Characters/Components/InteractComponent.h"
#include "Game/Characters/Components/HealthComponent.h"     
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/PostProcessComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Game/Characters/Components/PickupComponent.h"
#include "Game/Characters/Components/InventoryComponent.h"
#include "Game/Characters/Components/AnimationComponent.h"
#include "Game/Characters/Components/CharAudioComponent.h"
#include "Game/Characters/Components/CharCameraComponent.h"
#include "Game/Characters/Components/EquipComponent.h"
#include "Game/Characters/Components/ActionStateComponent.h"
#include "Game/Characters/Components/ItemVisualComponent.h"
#include "Game/Characters/Components/WeaponFireComponent.h"
#include "Game/Characters/Components/WeaponMeleeComponent.h"
#include "Game/Characters/Components/ThrowableComponent.h"
#include "Game/Characters/Components/SpikeComponent.h"
#include "Game/Characters/Components/RoleComponent.h"
#include "Game/Characters/Components/LagCompensationComponent.h"
#include "Shared/System/ItemsManager.h"
#include "Game/Data/CharacterAsset.h"
#include "Materials/MaterialInterface.h"
#include "Shared/Data/GlobalDataAsset.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Shared/Data/Items/ThrowableConfig.h"
#include "Shared/Data/Items/FirearmConfig.h"
#include "Game/Characters/Components/ItemUseComponent.h"
#include "Game/Modes/Zombie/ZombieMode.h"
#include "Game/Modes/Spike/Spike.h"
#include "Components/TextRenderComponent.h"
#include "Game/Subsystems/ActorManager.h"

const FVector ABaseCharacter::TPSMeshRelLoc(0.f, 0.f, -88.f);
const FRotator ABaseCharacter::TPSMeshRelRot(0.f, -90.f, 0.f);

// Sets default values
ABaseCharacter::ABaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

    FpsPivot = CreateDefaultSubobject<USceneComponent>(TEXT("FpsPivot"));
    FpsPivot->SetupAttachment(GetMesh());
	FpsPivot->SetRelativeRotation(FRotator(0, 90, 0));

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBloom"));
    CameraBoom->SetupAttachment(GetRootComponent());
    CameraBoom->bEnableCameraLag = true;

    CameraTps = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraTps"));
    CameraTps->SetupAttachment(CameraBoom);
    CameraTps->bUsePawnControlRotation = false;

    MeshFps = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshFps"));
    MeshFps->SetupAttachment(FpsPivot);

    CameraFps = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraFps"));
    CameraFps->bUsePawnControlRotation = false;
    CameraFps->SetupAttachment(FpsPivot);

	UE_LOG(LogTemp, Warning, TEXT("ABaseCharacter constructor called"));

    StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));

    InventoryComp = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
    InteractComp = CreateDefaultSubobject<UInteractComponent>(TEXT("InteractComponent"));
    HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    AnimationComp = CreateDefaultSubobject<UAnimationComponent>(TEXT("AnimationComponent"));
    AudioComp = CreateDefaultSubobject<UCharAudioComponent>(TEXT("AudioComponent"));
    CameraComp = CreateDefaultSubobject<UCharCameraComponent>(TEXT("CharCameraComponent"));
	EquipComp = CreateDefaultSubobject<UEquipComponent>(TEXT("EquipComponent"));
	ActionStateComp = CreateDefaultSubobject<UActionStateComponent>(TEXT("ActionStateComponent"));
	WeaponMeleeComp = CreateDefaultSubobject<UWeaponMeleeComponent>(TEXT("WeaponMeleeComponent"));
	SpikeComp = CreateDefaultSubobject<USpikeComponent>(TEXT("SpikeComponent"));
	RoleComp = CreateDefaultSubobject<URoleComponent>(TEXT("RoleComponent"));
	ItemUseComp = CreateDefaultSubobject<UItemUseComponent>(TEXT("ItemUseComponent"));
    ItemVisualComp = CreateDefaultSubobject<UItemVisualComponent>(TEXT("ItemVisualComponent"));
    WeaponFireComp = CreateDefaultSubobject<UWeaponFireComponent>(TEXT("WeaponFireComponent"));
    PickupComponent = CreateDefaultSubobject<UPickupComponent>(TEXT("PickupComponent"));
    ThrowableComp = CreateDefaultSubobject<UThrowableComponent>(TEXT("ThrowableComponent"));
	LagCompensationComp = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent"));

    CameraComp->Initialize(
        CameraFps,
        CameraTps,
        CameraBoom,
        MeshFps,
        GetMesh()
    );

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp) 
    {
        MoveComp->NavAgentProps.bCanCrouch = true;
        MoveComp->MaxWalkSpeedCrouched = CROUCH_WALK_SPEED;
	}

    if (GetMesh()) {
        GetMesh()->SetRelativeLocation(TPSMeshRelLoc);
        GetMesh()->SetRelativeRotation(TPSMeshRelRot);
    }
    if (FpsPivot) {
        // set same eye position
        BaseEyeHeight = 70;
        CrouchedEyeHeight = 70;
		FpsPivot->SetRelativeLocation(FVector(0.f, 0.f, -TPSMeshRelLoc.Z + BaseEyeHeight));
    }
    if (MeshFps)
    {
        MeshFps->SetRelativeLocation(FVector(0.f, 0.f, -170.f));
        MeshFps->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    }

    NameText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NameText"));
	NameText->SetVisibility(false); // hide by default, can be enabled for debugging or in specific game modes
    NameText->SetupAttachment(GetMesh()); // attach to character mesh (follows animations)

    NameText->SetHorizontalAlignment(EHTA_Center);
    NameText->SetVerticalAlignment(EVRTA_TextCenter);
    NameText->SetWorldSize(15.f);
    NameText->SetTextRenderColor(FColor::Yellow);

    // Place above head (tweak values for your skeleton)
    NameText->SetRelativeLocation(FVector(0.f, 0.f, 210.f));
   
    NameText->SetText(FText::FromString(TEXT("Player")));

    // Note: This allows physics assets to update on the server even when the mesh is not rendered,
    // but it may have a performance cost.
    // Consider refactoring to use capsules for hit detection (may look unnatural).
    GetMesh()->VisibilityBasedAnimTickOption =
        EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
    Super::BeginPlay();
    UGameManager* GameManager = UGameManager::Get(GetWorld());
    if (!GameManager) {
        UE_LOG(LogTemp, Warning, TEXT("GameManager is null in ABaseCharacter::BeginPlay"));
        return;
    }
	AActorManager* ActorMgr = AActorManager::Get(GetWorld());
    if (!ActorMgr) {
        UE_LOG(LogTemp, Warning, TEXT("ActorManager instance is null in ABaseCharacter::BeginPlay"));
        return;
    }
    CachedCharacterAsset = GameManager->CharacterAsset.Get();
    ActorMgr->RegisterPlayer(this);

	// setup, delegate bindings, etc.
	BindDelegates();
    SetupCrouchTimeline();
    SetupStunTimeline();
    SetupSpineKickTimeline();
    SetupPerception();
    SetupFlashPostProcess();
    SetupInitialInventory();

    BasePivotFpsZ = FpsPivot->GetRelativeLocation().Z;

    if (MeshFps) {
        if (UAnimInstance* FPSAnim = MeshFps->GetAnimInstance())
        {
            UE_LOG(LogTemp, Warning, TEXT("FPSAnim is valid in ABaseCharacter"));
            FPSAnim->OnPlayMontageNotifyBegin.AddDynamic(this, &ABaseCharacter::OnNotifyBegin);
        }
    }
    if (GetMesh()) {
        if (UAnimInstance* TPSAnim = GetMesh()->GetAnimInstance())
        {
            TPSAnim->OnPlayMontageNotifyBegin.AddDynamic(this, &ABaseCharacter::OnNotifyBegin);
        }
    }

    USkeletalMeshComponent* MeshMain = GetMesh();
    MeshMain->SetCollisionProfileName(TEXT("MyMesh"));

	AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>();
    if (GS)
    {
        if (GS->GetMatchMode() == EMatchMode::Spike)
        {
			SpikeComp->SetEnabled(true);
		}
	}
}

void ABaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (AActorManager* GM = AActorManager::Get(GetWorld()))
    {
        GM->UnregisterPlayer(this);
    }

    // clear timers
    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void ABaseCharacter::BindDelegates() { 
    if (EquipComp) {
        EquipComp->OnActiveItemChanged.AddUObject(
            this,
            &ABaseCharacter::UpdateCurrentWeapon
        );
        EquipComp->OnActiveItemChanged.AddUObject(
            WeaponMeleeComp,
            &UWeaponMeleeComponent::HandleActiveItemChanged
        );
        if (WeaponFireComp) {
            EquipComp->OnActiveItemChanged.AddUObject(
                WeaponFireComp,
                &UWeaponFireComponent::OnActiveItemChanged
            );
        }
        EquipComp->OnActiveItemChanged.Broadcast(EquipComp->GetActiveItemId());
        //this->UpdateCurrentWeapon(EquipComp->GetActiveItemId());
    }

    if (HealthComp)
    {
        HealthComp->OnDeath.AddUObject(this, &ABaseCharacter::OnHealthDepleted);
    }
    if (RoleComp)
    {
       RoleComp->OnRoleChanged.AddUObject(this, &ABaseCharacter::HandleRoleChanged);
       this->HandleRoleChanged(RoleComp->GetRole(), RoleComp->GetRole());
    }
}

void ABaseCharacter::UpdateCurrentWeapon(EItemId NewWeaponId)
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[%s] Updating current weapon to ItemId: %d"),
        HasAuthority() ? TEXT("Server") : TEXT("Client"),
        static_cast<uint8>(NewWeaponId)
    );
	auto ItemsMgr = UItemsManager::Get(GetWorld());
	
	const UItemConfig* ItemConfig = ItemsMgr->GetItemById(NewWeaponId);
    if (!ItemConfig) {
        UE_LOG(LogTemp, Warning, TEXT("ItemConfig is null in UpdateCurrentWeapon"));
		return;
	}

	// limit anim asset, so have to hardcode offset for firearm
    if (ItemConfig->GetItemType() == EItemType::Firearm) {
        const UFirearmConfig* FirearmConfig = Cast<UFirearmConfig>(ItemConfig);

        if (FirearmConfig && FirearmConfig->FirearmType == EFirearmType::Pistol) {
            MeshFps->SetRelativeLocation(FVector(-5.f, 10.f, -170.f));
        }
        else {
            MeshFps->SetRelativeLocation(FVector(0.f, 10.f, -170.f));
        }
    }
    else {
        MeshFps->SetRelativeLocation(FVector(0.f, 0.f, -170.f));
    }
    UpdateMaxWalkSpeed();
}

void ABaseCharacter::OnRep_PlayerState() {
    Super::OnRep_PlayerState();
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
   
    if (StunTimeline.IsPlaying()) {
        StunTimeline.TickTimeline(DeltaTime);
    }
    if (CrouchTimeline.IsPlaying()) {
        CrouchTimeline.TickTimeline(DeltaTime);
	}
    if (SpineKickTimeline.IsPlaying())
    {
        SpineKickTimeline.TickTimeline(DeltaTime);
    }

    if (FpsPivot)
    {
        FRotator AimRot;

        if (IsLocallyControlled() && Controller)
        {
            // Only the owning player drives their own view/aim
            AimRot = Controller->GetControlRotation();
        }
        else
        {
            // For spectators / remote clients: use the pawn's replicated aim, not the spectator's camera
            AimRot = GetBaseAimRotation(); // uses RemoteViewPitch for non-owned pawns
        }

        const FRotator TargetPivotRot(AimRot.Pitch, 90.f, 0.f);

        const FRotator Smoothed =
            FMath::RInterpTo(FpsPivot->GetRelativeRotation(), TargetPivotRot, DeltaTime, 20.f);

        FpsPivot->SetRelativeRotation(Smoothed);
    }
    // logic sound
    UpdateFootstepSound(DeltaTime);

    if (!IsNetMode(NM_DedicatedServer))
    {
        UpdateNameTextRotation();
    }
}

void ABaseCharacter::UpdateNameTextRotation()
{
    if (!NameText) return;
	if (!NameText->IsVisible()) return;

    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (!PC || !PC->PlayerCameraManager) return;

    const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
    const FVector TextLoc = NameText->GetComponentLocation();

    const FRotator LookAt = (CamLoc - TextLoc).Rotation();
    NameText->SetWorldRotation(FRotator(0.f, LookAt.Yaw, 0.f));
}

void ABaseCharacter::UpdateFootstepSound(float DeltaTime) {
    const float Speed = GetVelocity().Size2D();
   
    if (!CanPlayFootstep()) {
        return;
    }

    const float CurrentTime = GetWorld()->GetTimeSeconds();

    float FootstepInterval = 0.35f;
    if (Speed > 500.f) {
        FootstepInterval = 0.25f;
    }
    else if (Speed > 410.f) {
        FootstepInterval = 0.3f;
	}
    if (CurrentTime - LastFootstepTime >= FootstepInterval)
    {
        LastFootstepTime = CurrentTime;
		PlayFootstepSound();
    }
}

bool ABaseCharacter::CanPlayFootstep() const
{
    return GetVelocity().Size2D() > ABaseCharacter::FOOTSTEP_SPEED_MIN &&
        !GetCharacterMovement()->IsFalling();
}

void ABaseCharacter::Jump()
{
    if (GetCharacterMovement()->IsCrouching()) {
        UnCrouch();
        return;
	}
    Super::Jump();
}

void ABaseCharacter::StopJumping()
{
    Super::StopJumping();
}

void ABaseCharacter::RequestCrouch()
{
    if (GetCharacterMovement()->IsFalling()) {
        return;
    }
	UE_LOG(LogTemp, Warning, TEXT("RequestCrouch called"));
    Crouch();
}

void ABaseCharacter::RequestUnCrouch()
{
    UnCrouch();
}

void ABaseCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	CrouchToZ = BasePivotFpsZ - HalfHeightAdjust;
    CrouchTimeline.PlayFromStart();
    UpdateMaxWalkSpeed();
}

void ABaseCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
    CrouchToZ = BasePivotFpsZ;
    CrouchTimeline.PlayFromStart(); // forward again (not Reverse)
    UpdateMaxWalkSpeed();
}

void ABaseCharacter::HandleCrouchProgress(float Alpha)
{
    FVector Loc = FpsPivot->GetRelativeLocation();
    CurrentCrouchCompZ = FMath::Lerp(Loc.Z, CrouchToZ, Alpha);
    Loc.Z = CurrentCrouchCompZ;
    FpsPivot->SetRelativeLocation(Loc);
}
                                                                                                                         
void ABaseCharacter::ChangeView()
{
	if (CameraComp) {
        CameraComp->ToggleView();
	}
}

USkeletalMeshComponent* ABaseCharacter::GetCurrentMesh() const
{
    if (!CameraComp) {
		return GetMesh();
    }
    if (CameraComp->IsFPS())
    {
        return MeshFps;
    }
    else
    {
        return GetMesh();
    }
}

USkeletalMeshComponent* ABaseCharacter::GetMeshFps() const
{
    return MeshFps;
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ABaseCharacter, bIsAiming);
	DOREPLIFETIME(ABaseCharacter, SpeedMultiplier);
	DOREPLIFETIME(ABaseCharacter, CharacterSkin);
	DOREPLIFETIME(ABaseCharacter, bIsPermanentDead);
    DOREPLIFETIME_CONDITION(
        ABaseCharacter,
        CurrentMovementState,
        COND_SkipOwner
    );
}

void ABaseCharacter::RequestStartAiming()
{
	UE_LOG(LogTemp, Warning, TEXT("RequestStartAiming called"));
    if (!WeaponFireComp) {
        return;
	}
    if (!WeaponFireComp->CanWeaponAim()) {
		UE_LOG(LogTemp, Warning, TEXT("Cannot aim with current weapon"));
        return;
    }
    if (bIsAiming) {
        return;
    }
    
	// if is locally controlled, update immediately
    if (IsLocallyControlled()) {
		// Predictive update
		bIsAiming = true;
        if (CameraComp) CameraComp->SetAiming(true);
        UpdateMaxWalkSpeed();
	}

    ServerSetAiming(true);
}

void ABaseCharacter::RequestStopAiming()
{
    if (!bIsAiming) {
        return;
    }
  
    // if is locally controlled, update immediately
    if (IsLocallyControlled()) {
        bIsAiming = false;
        if (CameraComp) CameraComp->SetAiming(false);
        UpdateMaxWalkSpeed();
    }
    ServerSetAiming(false);
}


void ABaseCharacter::OnRep_IsAiming()
{
	UE_LOG(LogTemp, Warning, TEXT("OnRep_IsAiming: %s"), bIsAiming ? TEXT("true") : TEXT("false"));
    if (CameraComp)
        CameraComp->SetAiming(bIsAiming);
}

void ABaseCharacter::ServerSetAiming_Implementation(bool bNewAiming)
{
    if (!IsAlive()) {
        return;
    }

    if (!WeaponFireComp) {
        return;
    }
    if (bNewAiming && !WeaponFireComp->CanWeaponAim()) {
        return;
	}

    if (bIsAiming == bNewAiming) {
        return; // no change
    }
	bIsAiming = bNewAiming;
	OnRep_IsAiming();

    UpdateMaxWalkSpeed();
}


float ABaseCharacter::GetSpeedWalkRatio() const
{
    float Ratio = 1.0f;

    if (!EquipComp) {
		return Ratio;
    }
    // get active weapon data
	const UItemConfig* Config = EquipComp->GetActiveItemConfig();
    if (!Config) {
        return Ratio;
    }

    // Aiming penalty
    if (bIsAiming)
    {
        Ratio *= 0.2f;
    }

    return Ratio;
}


void ABaseCharacter::UpdateMaxWalkSpeed() {
	UE_LOG(LogTemp, Warning, TEXT("UpdateMaxWalkSpeed called"));
    float Speed = 0;
    if (IsCrouched()) {
        Speed = CROUCH_WALK_SPEED;
    }
    else if (CurrentMovementState == EMovementState::Slow) {
        Speed = SLOW_WALK_SPEED;
    }
    else {
        Speed = NORMAL_WALK_SPEED;
		// sub weight penalty
        const UItemConfig* Config = EquipComp ? EquipComp->GetActiveItemConfig() : nullptr;
        if (Config) {
            Speed -= Config->Weight;
        }

        if (bIsAiming) {
            Speed *= 0.2f; // aiming penalty
        }

        if (RoleComp) {
            if (RoleComp->GetRole() == ECharacterRole::Zombie) {
                Speed += 10.f; // zombie role moves faster
			}
        }
    }
	Speed *= SpeedMultiplier;
	GetCharacterMovement()->MaxWalkSpeed = Speed;
}

float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority()) {
        return 0.f; // only server handles damage
    }

	// ask for game mode if damage is allowed 
    AShooterGameMode* GM = Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM) {
        UE_LOG(LogTemp, Warning, TEXT("GameMode is null in ABaseCharacter::TakeDamage"));
        return 0.f;
    }
    if (!GM->IsDamageAllowed(EventInstigator, this->Controller)) {
        return 0.f;
	}

    if (HealthComp && HealthComp->IsDead())
    {
        return 0.f; // already dead
	}
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
  
    const FArmorState& Armor = InventoryComp->GetArmorState();
    if (Armor.ArmorPoints > 0)
    {
        float DamageToHealth = ActualDamage * Armor.ArmorRatio;
        float DamageToArmor = (ActualDamage - DamageToHealth) * Armor.ArmorEfficiency;

        const float ArmorRandMin = 0.9f;
        const float ArmorRandMax = 1.1f;
        const float ArmorRandMul = FMath::FRandRange(ArmorRandMin, ArmorRandMax);
        DamageToArmor *= ArmorRandMul;

		int32 NewArmorPoints = Armor.ArmorPoints;
        if (DamageToArmor >= Armor.ArmorPoints)
        {
            float Overflow = DamageToArmor - Armor.ArmorPoints;
            DamageToHealth += Overflow;
            NewArmorPoints = 0;
        }
        else
        {
            NewArmorPoints -= DamageToArmor;
        }

        ActualDamage = DamageToHealth;

        InventoryComp->UpdateArmorPoints(NewArmorPoints); // update armor state
	}
	UE_LOG(LogTemp, Warning, TEXT("ABaseCharacter::TakeDamage after armor: %f"), ActualDamage);
    LastHitByController = EventInstigator;
	bLastHitWasHeadshot = false;
    if (EventInstigator)
    {
        DamageInstigators.AddUnique(EventInstigator);
    }

    if (DamageEvent.IsOfType(FMyPointDamageEvent::ClassID))
    {
        const FMyPointDamageEvent* MyEvent =
            static_cast<const FMyPointDamageEvent*>(&DamageEvent);

        EItemId WeaponId = static_cast<EItemId>(MyEvent->WeaponID);

        UE_LOG(LogTemp, Warning, TEXT("Damage came from WeaponId: %d"), (int32)WeaponId);

   
		LastDamageCauser = UItemsManager::Get(GetWorld())->GetItemById(WeaponId);
		bLastHitWasHeadshot = MyEvent->bIsHeadshot;
    }
    else if (DamageCauser) {
        if (const AThrownProjectile* Projectile = Cast<AThrownProjectile>(DamageCauser))
        {
            LastDamageCauser = Projectile->GetWeaponData();
        }
        else if (DamageCauser->IsA(ASpike::StaticClass()))
        {
            LastDamageCauser =
                UItemsManager::Get(GetWorld())->GetItemById(EItemId::SPIKE);
        }
    }
    if (!HealthComp) {
        UE_LOG(LogTemp, Warning, TEXT("HealthComp is null in TakeDamage"));
		return ActualDamage;
	}

    HealthComp->ApplyDamage(ActualDamage);
    ClientPlayHitEffect();

	// if bot, notify AI perception
    if (IsBot()) {
        if (ABotAIController* AICon = Cast<ABotAIController>(GetController()))
        {
            if (AICon->GetTargetActor() == nullptr)
            {
                APawn* Target = EventInstigator ? EventInstigator->GetPawn() : nullptr;
                if (Target) {
                    AICon->SetFocalPoint(Target->GetActorLocation());
                }
            }
        }
    }

	ApplyHitSlow(0.5, 0.25f); // slow for 0.25s at 20% speed
    return ActualDamage;
}


void ABaseCharacter::OnMeleeNotify()
{
    
}


void ABaseCharacter::OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	UE_LOG(LogTemp, Warning, TEXT("OnNotifyBegin called with NotifyName: %s"), *NotifyName.ToString());
    if (NotifyName == "MeleeAttack")
    {
        OnMeleeNotify();
    }
    else if (NotifyName == "Notify_GrabMag")
    {
        if (ItemVisualComp)
        {
            ItemVisualComp->OnNotifyGrabMag();
        }
	}
    else if (NotifyName == "Notify_InsertMag")
    {
        if (ItemVisualComp)
        {
            ItemVisualComp->OnNotifyInsertMag();
        }
	}
    else if (NotifyName == "Throw_NadeRelease")
    {
        if (ThrowableComp)
        {
            ThrowableComp->OnNadeRelease();
        }
	}
}

// When health reaches zero, soldier -> become zombie
void ABaseCharacter::OnHealthDepleted()
{
    // Play animation, ragdoll, notify game mode, etc.
	UE_LOG(LogTemp, Warning, TEXT("Character has died."));
    if (HasAuthority())
    {
        AShooterGameMode* GM = Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this));
        if (!GM) {
            return;
        }
        GM->HandleCharacterKilled(LastHitByController.Get(), DamageInstigators, this, LastDamageCauser.Get(), bLastHitWasHeadshot);
        LastHitByController = nullptr; // reset after use
        LastDamageCauser = nullptr; // reset after use
        DamageInstigators.Reset();

        if (bIsAiming) {
            bIsAiming = false;
            if (CameraComp) CameraComp->SetAiming(false);
		}
    }
}

// server function to apply death effects
void ABaseCharacter::ApplyRealDeath(bool bDropInventory, bool bInPermanentDead)
{
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();

    if (SpikeComp && SpikeComp->IsEnabled()) {
		SpikeComp->OnOwnerDead();
    }

    // disable action state
    if (ActionStateComp)
    {
		ActionStateComp->ForceSetState(EActionState::Disabled);
	}

    if (AAIController* AI = Cast<AAIController>(GetController()))
    {
        if (UBrainComponent* Brain = AI->GetBrainComponent())
        {
            Brain->StopLogic(TEXT("Bot died"));
        }
    }

    if (bDropInventory && InventoryComp)
    {
        InventoryComp->DropAllItems();
    }
    if (EquipComp)
    {
        EquipComp->UnequipCurrentItem();
	}

    MulticastCharacterDeath(); 
    DisableDeadMeshTick();
    if (bIsPermanentDead != bInPermanentDead) {
        bIsPermanentDead = bInPermanentDead;
        OnRep_IsPermanentDead();
    }
}

void ABaseCharacter::MulticastCharacterDeath_Implementation()
{
	// log server or client
	UE_LOG(LogTemp, Warning, TEXT("MulticastPlayerDeath called on %s"), HasAuthority() ? TEXT("Server") : TEXT("Client"));
   
    UCapsuleComponent* Capsule = GetCapsuleComponent();
    USkeletalMeshComponent* SkelMesh = GetMesh();
    if (!Capsule || !SkelMesh)
        return;

    Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SkelMesh->SetCollisionProfileName(TEXT("Ragdoll"));
    SkelMesh->SetSimulatePhysics(true);

    // if is hero, play sound hero dead
    if (GetCharacterRole() == ECharacterRole::Hero) {
        if (AudioComp) {
            AudioComp->PlayHeroDeath();
        }
    }
    else if (GetCharacterRole() == ECharacterRole::Zombie) {
        if (AudioComp) {
            AudioComp->PlayZombieDeath();
        }
	}
    else {
        if (AudioComp) {
            AudioComp->PlaySoldierDeath();
        }
    }
    AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
    if (!MyPC) {
        return;
	}
	bool bIsLocalPlayer = MyPC->GetViewTarget() == this;

	auto LocalPC = Cast<AMyPlayerController>(GetController());
    if (LocalPC) {
		auto CurTime = GetWorld()->GetTimeSeconds();
        LocalPC->SetTimeOfDeath(CurTime);
    }
    if (bIsLocalPlayer) {
        MyPC->SetIgnoreLookInput(true);
        MyPC->SetIgnoreMoveInput(true);
        
        if (CameraComp->IsFPS()) {
            ChangeView(); // switch to tps view
        }
       
        // Use current camera position as start
        const FVector CamLoc = CameraTps->GetComponentLocation();
        const FRotator CamRot = CameraTps->GetComponentRotation();

        // request view
        TWeakObjectPtr<AMyPlayerController> WeakPC(MyPC);

		GetWorld()->GetTimerManager().ClearTimer(SpectatorViewTimer);
        GetWorld()->GetTimerManager().SetTimer(
            SpectatorViewTimer,
            [WeakPC, this]()
            {
                if (!WeakPC.IsValid()) return;

                if (WeakPC->GetViewTarget() != this) {
                    // already switched view, maybe by user
                    return;
                }

                if (WeakPC->IsSpectatingState())
                {
                    WeakPC->RequestSpectateNextPlayer();
                }
            },
            2.0f,
            false
        );
    }
}

void ABaseCharacter::ClientPlayHitEffect_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ClientPlayHitEffect called"));
	OnHit.Broadcast();
}

void ABaseCharacter::PlayBloodFx(const FVector& HitLocation, const FVector& HitNormal)
{
    PlayEffectHitReact();
    if (IsFpsViewMode()) {
		return; // no blood fx in fps mode
    }
    if (!CachedCharacterAsset) {
        return;
    }
    const FRotator HitRotation = HitNormal.Rotation();

	auto CharRole = GetCharacterRole();
    if (CharRole != ECharacterRole::Zombie) {
        if (CachedCharacterAsset->BloodFx) {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                CachedCharacterAsset->BloodFx,
                HitLocation
            );
        }
    }
    else {
        if (CachedCharacterAsset->HitFx)
        {
            FVector EffectScale(0.1f); // scale
            FName SocketName = NAME_None; // or socket name if needed

            UGameplayStatics::SpawnEmitterAttached(
                CachedCharacterAsset->HitFx,
                GetMesh(),                 // USceneComponent* to attach to
                SocketName,
                HitLocation,                  // relative location
                HitRotation,         // relative rotation
                EffectScale,                  // relative scale
                EAttachLocation::KeepWorldPosition,
                true                           // auto destroy
            );
        }
    }
    if (AudioComp) {
        //   AudioComp->PlayBloodHit();
	}
}

void ABaseCharacter::PlayStunEffect(const float& Strength) 
{
	UE_LOG(LogTemp, Warning, TEXT("PlayStunEffect called with Strength: %f"), Strength);
    if (!CachedCharacterAsset) {
        return;
	}
    if (!FlashMID) {
        return;
    }
	
    if (CachedCharacterAsset->StunCurve)
    {
        float NewDuration = BaseStunDuration * Strength;
        // Restart the timeline from the beginning
        StunTimeline.SetPlayRate(BaseStunDuration / FMath::Max(NewDuration, 0.01f));
        StunTimeline.PlayFromStart();
    }
}

void ABaseCharacter::OnStunTimelineUpdate(float Value)
{
	// only if local owner is viewing this character
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!FlashMID || !PC || PC->GetViewTarget() != this)
        return;

    FlashMID->SetScalarParameterValue(
        TEXT("Intensity"),
        Value
    );
}

void ABaseCharacter::OnStunTimelineFinished()
{
    if (FlashMID) {
        FlashMID->SetScalarParameterValue(
            TEXT("Intensity"),
            0.f
        );
	}
}

float ABaseCharacter::GetAimSensitivity() const {
    if (CameraComp) {
        return CameraComp->GetAimSensitivity();
    }
	return 1.0f;
}

void ABaseCharacter::OnPlantSpikeStarted() {
    // play sound
    if (AudioComp) {
        AudioComp->PlayPlantSpike();
    }
}

void ABaseCharacter::OnPlantSpikeStopped() {
    if (AudioComp) {
        AudioComp->StopPlantSpike();
	}
}

void ABaseCharacter::OnDefuseSpikeStarted() {
    if (AudioComp) {
        AudioComp->PlayDefuseSpike();
	}
}

void ABaseCharacter::OnDefuseSpikeStopped() {
    if (AudioComp) {
        AudioComp->StopDefuseSpike();
    }
}

void ABaseCharacter::Destroyed()
{
    Super::Destroyed();

    // log client or server
	UE_LOG(LogTemp, Warning, TEXT("ABaseCharacter::Destroyed called on %s"), HasAuthority() ? TEXT("Server") : TEXT("Client"));
    
    if (ItemVisualComp)
    {
        ItemVisualComp->OnOwnerDead();
    }
}

void ABaseCharacter::ServerSetIsSlow_Implementation(bool bNewIsSlow)
{
    if (!IsAlive())
    {
        return;
    }
	UE_LOG(LogTemp, Warning, TEXT("ServerSetIsSlow called with bNewIsSlow: %s"), bNewIsSlow ? TEXT("true") : TEXT("false"));
	CurrentMovementState = bNewIsSlow ? EMovementState::Slow : EMovementState::Normal;
    UpdateMaxWalkSpeed();
}

void ABaseCharacter::OnRep_CurrentMovementState()
{
    if (IsLocallyControlled()) {
		return; // already handled locally
    }
    UpdateMaxWalkSpeed();
}

void ABaseCharacter::PlayFootstepSound()
{
    if (!AudioComp) {
        UE_LOG(LogTemp, Warning, TEXT("AudioComp is null in PlayFootstepSound"));
        return;
    }

    const float Speed = GetVelocity().Size2D();
    const bool bGrounded = !GetCharacterMovement()->IsFalling();

    // OFF when slow
    if (Speed < ABaseCharacter::FOOTSTEP_SPEED_MIN)
    {
        return;
    }

    if (!bGrounded)
    {
        return;
    }

    // Play at actor's feet
	AudioComp->PlayFootstep();
}

bool ABaseCharacter::IsAlive() const
{
    if (HealthComp)
    {
        return !HealthComp->IsDead();
    }
    return true; // if no health component, assume alive
}

bool ABaseCharacter::IsDead() const
{
    if (HealthComp)
    {
        return HealthComp->IsDead();
    }
    return false; // if no health component, assume alive
}

void ABaseCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);
    PlayLandingSound();
}

void ABaseCharacter::PlayLandingSound()
{
    if (AudioComp) {
        AudioComp->PlayLanding();
        return;
    }
}

void ABaseCharacter::BecomeViewTarget(APlayerController* PC)
{
    Super::BecomeViewTarget(PC);
    if (!PC->IsLocalController()) {
        UE_LOG(LogTemp, Warning, TEXT("ABaseCharacter::BecomeViewTarget called - NO LOCAL"));
        return;
    }

    AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC);
    if (!MyPC)
    {
        UE_LOG(LogTemp, Error, TEXT("PC is not AMyPlayerController in BecomeViewTarget"));
        return;
    }
    MyPC->BindCharacter(this);
  
    if (IsValid(CameraComp)) {
        CameraComp->OnBecomeViewTarget(MyPC);
    }
}

void ABaseCharacter::EndViewTarget(APlayerController* PC)
{
    Super::EndViewTarget(PC);

    if (!IsLocallyControlled() && PC && PC->IsLocalController())
    {
		//SetFpsView(false);
    }
    if (CameraComp) {
        CameraComp->OnEndViewTarget(PC);
	}
}

void ABaseCharacter::ApplyRotationMode()
{
    auto* Move = GetCharacterMovement();
    if (!Move) return;
   
    if (IsBot())
    {
		/*Move->bOrientRotationToMovement = true;
        bUseControllerRotationYaw = true;*/
    }
}

void ABaseCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    ApplyRotationMode();
    UpdateInteractComponentTick();
}

void ABaseCharacter::OnRep_Controller()
{
    Super::OnRep_Controller();

    ApplyRotationMode();
    UpdateInteractComponentTick();
}

FVector ABaseCharacter::GetThrowableLocation() const
{
    FVector EyeLoc;
    FRotator EyeRot;
    GetActorEyesViewPoint(EyeLoc, EyeRot);

    return EyeLoc
        + GetActorRightVector() * 10.f
        + FVector(0.f, 0.f, 30.f);
}

EMovementState ABaseCharacter::GetCurrentMovementState() const {
    return CurrentMovementState;
}

bool ABaseCharacter::IsFpsViewMode() const {
    if (CameraComp) {
        return CameraComp->IsFPS();
	}
	return false;
}

bool ABaseCharacter::IsAiming() const {
    return bIsAiming;
}

UPickupComponent* ABaseCharacter::GetPickupComponent() const {
	return PickupComponent.Get();
}

UInventoryComponent* ABaseCharacter::GetInventoryComponent() const {
    return InventoryComp.Get();
}

UInteractComponent* ABaseCharacter::GetInteractComponent() const {
    return InteractComp.Get();
}

UHealthComponent* ABaseCharacter::GetHealthComponent() const {
    return HealthComp.Get();
}

UAnimationComponent* ABaseCharacter::GetAnimationComponent() const {
    return AnimationComp.Get();
}

UEquipComponent* ABaseCharacter::GetEquipComponent() const {
    return EquipComp.Get();
}

UWeaponMeleeComponent* ABaseCharacter::GetWeaponMeleeComponent() const {
    return WeaponMeleeComp.Get();
}

ULagCompensationComponent* ABaseCharacter::GetLagCompensationComponent() const {
    return LagCompensationComp.Get();
}

UActionStateComponent* ABaseCharacter::GetActionStateComponent() const {
    return ActionStateComp.Get();
}

UItemVisualComponent* ABaseCharacter::GetItemVisualComponent() const {
    return ItemVisualComp.Get();
}

UCharAudioComponent* ABaseCharacter::GetAudioComponent() const {
    return AudioComp.Get();
}

UWeaponFireComponent* ABaseCharacter::GetWeaponFireComponent() const {
    return WeaponFireComp.Get();
}

URoleComponent* ABaseCharacter::GetRoleComponent() const {
    return RoleComp.Get();
}

UCharCameraComponent* ABaseCharacter::GetCharCameraComponent() const {
    return CameraComp.Get();
}

void ABaseCharacter::RequestSlowMovement(bool bNewIsSlow)
{
    ServerSetIsSlow(bNewIsSlow);

    if (IsLocallyControlled())
    {
        CurrentMovementState = bNewIsSlow
            ? EMovementState::Slow
            : EMovementState::Normal;

        UpdateMaxWalkSpeed();
    }
}

void ABaseCharacter::RequestJump() {
    if (GetCharacterMovement()->IsFalling())
    {
        return;
    }

	Jump();
}

bool ABaseCharacter::IsBot() const
{
    return Controller && Controller->IsA<AAIController>();
}

EEquippedAnimState ABaseCharacter::GetEquippedAnimState() const
{
    if (EquipComp) {
		return EquipComp->GetEquippedAnimState();
	}

    return EEquippedAnimState::Unarmed;
}

UThrowableComponent* ABaseCharacter::GetThrowableComponent() const {
    return ThrowableComp.Get();
}

USpikeComponent* ABaseCharacter::GetSpikeComponent() const {
    return SpikeComp.Get();
}

void ABaseCharacter::ApplyDefaultsForRole(ECharacterRole NewRole) {
    // this should happens on server only
    if (NewRole == ECharacterRole::Zombie)
    {
        if (HealthComp) {
            HealthComp->SetMaxHealth(FGameConstants::INIT_HEALTH_ZOMBIE);
            HealthComp->ResetHealth();
        }
    }
    else if (NewRole == ECharacterRole::Hero)
    {
        if (HealthComp) {
            HealthComp->SetMaxHealth(FGameConstants::INIT_HEALTH_HERO);
            HealthComp->ResetHealth();
        }
    }
    else // human
    {
        if (HealthComp) {
            HealthComp->SetMaxHealth(FGameConstants::INIT_HEALTH_SOLIDER);
            HealthComp->ResetHealth();
        }
    }

    if (ActionStateComp) {
        ActionStateComp->ForceSetState(EActionState::Idle);
    }

    if (NewRole == ECharacterRole::Hero) {
        if (InventoryComp) {
            InventoryComp->OnBecomeHero();
        }
        if (EquipComp) {
            EquipComp->SelectSlot(FGameConstants::SLOT_MELEE);
        }
    }
    else if (NewRole == ECharacterRole::Zombie) {
        if (InventoryComp) {
            InventoryComp->OnBecomeZombie();
        }
        if (EquipComp) {
            EquipComp->SelectSlot(FGameConstants::SLOT_MELEE);
        }
    }
}

void ABaseCharacter::HandleRoleChanged(ECharacterRole OldRole, ECharacterRole NewRole)
{
    // Gameplay wiring (attach/detach components, set attack provider, etc.)
    ApplyLoadoutByRole(NewRole);

    // Presentation
    ApplyVisualByRole(NewRole);
    if (this->HasAuthority())
    {
		UE_LOG(LogTemp, Warning, TEXT("Applying defaults for new role: %d"), (int32)NewRole);
        ApplyDefaultsForRole(NewRole);
    }

    // play effect if human to zombie
    if (NewRole == ECharacterRole::Zombie)
    {
        PlayZombieSpawnEffects();
    }
    else if (NewRole == ECharacterRole::Hero) { 
        if (AudioComp) {
            AudioComp->PlayHeroSpawn();
        }
    }

	// role changed mean update walk speed
	UpdateMaxWalkSpeed();
}

void ABaseCharacter::ApplyVisualByRole(ECharacterRole NewRole)
{
	UE_LOG(LogTemp, Warning, TEXT("Applying visuals for role: %d"), (int32)NewRole);
    if (!CachedCharacterAsset)
    {
        return;
    }
    UCharacterVisualSet* VisualSet = nullptr;

    if (NewRole == ECharacterRole::Zombie)
    {
		VisualSet = CachedCharacterAsset->ZombieVisualSet;
    }
    else if (NewRole == ECharacterRole::Hero)
    {
		VisualSet = CachedCharacterAsset->HeroVisualSet;
        if (CachedCharacterAsset->HeroFx)
        {
            FVector EffectScale(0.4f); // scale
            FName SocketName = NAME_None; // or socket name if needed
			FVector Loc = GetActorLocation();
			Loc.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

            UGameplayStatics::SpawnEmitterAttached(
                CachedCharacterAsset->HeroFx,
                GetMesh(),                 // USceneComponent* to attach to
                SocketName,
                Loc,                  // relative location
                FRotator::ZeroRotator,         // relative rotation
                EffectScale,                  // relative scale
                EAttachLocation::KeepWorldPosition,
                true                           // auto destroy
            );
        }
    }
    else
    {
        VisualSet = CachedCharacterAsset->SoldierVisualSet;
        if (CharacterSkin == FGameConstants::SKIN_CHARACTER_ATTACKER) {
            VisualSet = CachedCharacterAsset->SoldierVisualSet;
        }
        else if (CharacterSkin == FGameConstants::SKIN_CHARACTER_DEFENDER) {
            VisualSet = CachedCharacterAsset->SoldierVisualSet2;
        }
    }

    // Apply TPS
    if (USkeletalMeshComponent* TpsMesh = GetMesh())
    {
        if (VisualSet->TpsMesh && TpsMesh->GetSkeletalMeshAsset() != VisualSet->TpsMesh)
        {
            TpsMesh->SetSkeletalMesh(VisualSet->TpsMesh);
        }

        if (VisualSet->TpsAnimClass)
        {
            TpsMesh->SetAnimInstanceClass(VisualSet->TpsAnimClass);
        }
    }
    if (!VisualSet) {
        return;
    }

    // Apply FPS
    if (MeshFps)
    {
        if (VisualSet->FpsMesh && MeshFps->GetSkeletalMeshAsset() != VisualSet->FpsMesh)
        {
            MeshFps->SetSkeletalMesh(VisualSet->FpsMesh);
        }
        if (VisualSet->FpsAnimClass)
        {
            MeshFps->SetAnimInstanceClass(VisualSet->FpsAnimClass);
        }
    }

    // Re-bind montage notify if needed (because anim class may change)
    if (MeshFps)
    {
        if (UAnimInstance* FPSAnim = MeshFps->GetAnimInstance())
        {
            FPSAnim->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ABaseCharacter::OnNotifyBegin);
            FPSAnim->OnPlayMontageNotifyBegin.AddDynamic(this, &ABaseCharacter::OnNotifyBegin);
        }
    }
    if (GetMesh())
    {
        if (UAnimInstance* TPSAnim = GetMesh()->GetAnimInstance())
        {
            TPSAnim->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ABaseCharacter::OnNotifyBegin);
            TPSAnim->OnPlayMontageNotifyBegin.AddDynamic(this, &ABaseCharacter::OnNotifyBegin);
        }
    }

    // -------- Visual reset --------
    if (ItemVisualComp)
    {
        if (NewRole == ECharacterRole::Zombie)
        {
            ItemVisualComp->OnOwnerDead();
        }
        else
        {

        }
    }
}

void ABaseCharacter::ApplyLoadoutByRole(ECharacterRole NewRole)
{
    if (NewRole == ECharacterRole::Zombie)
    {
        if (WeaponFireComp)
        {
            WeaponFireComp->SetEnabled(false);
		}

        if (ThrowableComp) {
			ThrowableComp->SetEnabled(false);
        }

        if (PickupComponent) {
            PickupComponent->SetEnabled(false);
        }
    }
    else if (NewRole == ECharacterRole::Hero)
    {
        if (WeaponFireComp)
        {
            WeaponFireComp->SetEnabled(false);
		}

        if (PickupComponent) {
            PickupComponent->SetEnabled(false);
		}

        if (ThrowableComp) {
            ThrowableComp->SetEnabled(true);
		}
    }
    else
    {
        // human
	}
}


bool ABaseCharacter::IsCharacterRole(ECharacterRole InRole) const
{
    if (RoleComp)
    {
        return RoleComp->GetRole() == InRole;
    }
    return false;
}

void ABaseCharacter::ApplyHitSlow(float Multiplier, float Duration)
{
    Multiplier = FMath::Clamp(Multiplier, 0.1f, 1.0f);

    // Choose your stacking rule:
    // "More slow wins" (keep the smallest multiplier).
    SpeedMultiplier = FMath::Min(SpeedMultiplier, Multiplier);

    // Apply immediately on server too.
    OnRep_SpeedMultiplier();

    // Refresh timer each time you get hit.
    GetWorldTimerManager().ClearTimer(HitSlowTimer);
    GetWorldTimerManager().SetTimer(HitSlowTimer, this, &ABaseCharacter::ClearHitSlow, Duration, false);
}

void ABaseCharacter::ClearHitSlow()
{
    SpeedMultiplier = 1.0f;
    OnRep_SpeedMultiplier();
}

void ABaseCharacter::OnRep_SpeedMultiplier()
{
	UpdateMaxWalkSpeed();
}

ETeamId ABaseCharacter::GetTeamId() const
{
    if (const AMyPlayerState* PS = GetPlayerState<AMyPlayerState>())
    {
        return PS->GetTeamId();
    }

    return ETeamId::None;
}

void ABaseCharacter::RequestPrimaryActionPressed() {
    if (!CanAct()) {
        return;
    }
    if (ItemUseComp) {
        ItemUseComp->PrimaryPressed();
    }
}

void ABaseCharacter::RequestPrimaryActionReleased() {
    if (!CanAct()) {
        return;
    }
    if (ItemUseComp) {
        ItemUseComp->PrimaryReleased();
    }
}

void ABaseCharacter::RequestSecondaryActionPressed() {
    if (!CanAct()) {
        return;
    }
    if (ItemUseComp) {
        ItemUseComp->SecondaryPressed();
    }
}

void ABaseCharacter::RequestSecondaryActionReleased() {
    if (!CanAct()) {
        return;
    }
    if (ItemUseComp) {
        ItemUseComp->SecondaryReleased();
    }
}

void ABaseCharacter::RequestReloadPressed() {
    if (!CanAct()) {
        return;
    }
    if (ItemUseComp) {
        ItemUseComp->ReloadPressed();
    }
}

bool ABaseCharacter::CanAct() {
    if (!IsAlive()) return false;
    return true;
}

void ABaseCharacter::SetupCrouchTimeline()
{
	if (!CachedCharacterAsset) return;
    if (!CachedCharacterAsset->CrouchCurve) return;

    FOnTimelineFloat Update;
    Update.BindUFunction(this, FName("HandleCrouchProgress"));
    CrouchTimeline.AddInterpFloat(CachedCharacterAsset->CrouchCurve, Update);
    CrouchTimeline.SetLooping(false);
}

void ABaseCharacter::SetupStunTimeline()
{
    if (!CachedCharacterAsset || !CachedCharacterAsset->StunCurve) return;

    FOnTimelineFloat UpdateFunction;
    UpdateFunction.BindUFunction(this, FName("OnStunTimelineUpdate"));
    StunTimeline.AddInterpFloat(CachedCharacterAsset->StunCurve, UpdateFunction);

    FOnTimelineEvent FinishedFunction;
    FinishedFunction.BindUFunction(this, FName("OnStunTimelineFinished"));
    StunTimeline.SetTimelineFinishedFunc(FinishedFunction);

    StunTimeline.SetLooping(false);
    BaseStunDuration = StunTimeline.GetTimelineLength();
}

void ABaseCharacter::SetupPerception()
{
    if (!StimuliSource) return;

    StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
    StimuliSource->RegisterWithPerceptionSystem();
}

void ABaseCharacter::SetupFlashPostProcess()
{
    if (!CachedCharacterAsset || !CachedCharacterAsset->FlashPPMat) return;

    FlashMID = UMaterialInstanceDynamic::Create(CachedCharacterAsset->FlashPPMat, this);

    // Apply to the active view camera (FPS here; if you can switch, apply to both)
    if (CameraFps)
    {
        CameraFps->PostProcessSettings.AddBlendable(FlashMID, 1.0f);
    }
    if (CameraTps)
    {
        CameraTps->PostProcessSettings.AddBlendable(FlashMID, 1.0f);
    }

    FlashMID->SetScalarParameterValue(TEXT("Intensity"), 0.f);
}

bool ABaseCharacter::IsSpikeMode() const
{
    const AShooterGameState* GS = GetWorld() ? GetWorld()->GetGameState<AShooterGameState>() : nullptr;
    return GS && (GS->GetMatchMode() == EMatchMode::Spike);
}


ECharacterRole ABaseCharacter::GetCharacterRole() const {
    if (RoleComp) {
        return RoleComp->GetRole();
    }
    return ECharacterRole::Human;
}

void ABaseCharacter::RequestBecomeHero() {
    if (!HasAuthority()) {
        ServerBecomeHero();
        return;
    }
    BecomeHero_Internal();
}

void ABaseCharacter::ServerBecomeHero_Implementation() {
    BecomeHero_Internal();
}

void ABaseCharacter::BecomeHero_Internal() {
    if (!RoleComp) {
        return;
    }
    if (RoleComp->GetRole() == ECharacterRole::Hero) {
        return;
    }

    AZombieMode* ZM = GetWorld()->GetAuthGameMode<AZombieMode>();
    if (ZM)
    {
        ZM->BecomeHero(this);
    }
}

void ABaseCharacter::SetupInitialInventory()
{
    if (!HasAuthority()) return;

    if (InventoryComp)
    {
        InventoryComp->InitBasicWeapon();
    }
    if (EquipComp)
    {
        EquipComp->AutoSelectBestWeapon();
    }

    UE_LOG(LogTemp, Warning, TEXT("BeginPlay: Added test items to inventory"));
}

void ABaseCharacter::MulticastRevive_Implementation()
{
    // reset collision and physics
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetMesh()->SetSimulatePhysics(false);
    GetMesh()->SetCollisionProfileName(TEXT("MyMesh"));

    GetMesh()->SetRelativeLocation(TPSMeshRelLoc);
    GetMesh()->SetRelativeRotation(TPSMeshRelRot);

    PlayZombieSpawnEffects();

	AMyPlayerController* PC = Cast<AMyPlayerController>(GetController());
    if (PC) {
        PC->SetIgnoreLookInput(false);
        PC->SetIgnoreMoveInput(false);
        PC->SetViewTargetWithBlend(this, 0.1f);

        if (!CameraComp->IsFPS()) {
            ChangeView(); // switch to tps view
        }
    }
    if (DeathCameraProxy.IsValid()) {
        DeathCameraProxy->Destroy();
        DeathCameraProxy = nullptr;
	}

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->SetMovementMode(MOVE_Walking);
		UpdateMaxWalkSpeed();
    }
}

void ABaseCharacter::Revive()
{
    if (!HasAuthority()) return;
    MulticastRevive();

    if (UHealthComponent* HC = FindComponentByClass<UHealthComponent>())
        HC->SetHealth(HC->GetMaxHealth());

    AAIController* AI = Cast<AAIController>(GetController());
    if (AI)
    {
        UBrainComponent* Brain = AI->GetBrainComponent();
        if (Brain)
        {
            Brain->StartLogic();
        }
    }

    if (ActionStateComp)
    {
        ActionStateComp->ForceSetState(EActionState::Idle);
    }

    if (EquipComp)
    {
        EquipComp->AutoSelectBestWeapon();
	}
}

void ABaseCharacter::PlayZombieSpawnEffects() {
    if (AudioComp) {
        AudioComp->PlayZombieSpawn();
    }
    FVector SpawnLocation = FVector::ZeroVector;
    SpawnLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    if (CachedCharacterAsset && CachedCharacterAsset->TurnToZombieFx)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            CachedCharacterAsset->TurnToZombieFx,
            GetRootComponent(),
            NAME_None,
            SpawnLocation,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            true
        );
    }
}

bool ABaseCharacter::IsHero() const {
    return GetCharacterRole() == ECharacterRole::Hero;
}

bool ABaseCharacter::IsZombie() const {
    return GetCharacterRole() == ECharacterRole::Zombie;
}

FString ABaseCharacter::GetPlayerName() const {
    if (const AMyPlayerState* PS = GetPlayerState<AMyPlayerState>())
    {
        return PS->GetPlayerName();
    }
    static const FString DefaultName = TEXT("Unknown");
	return DefaultName;
}

bool ABaseCharacter::IsWorkingWithSpike() const {
    if (ActionStateComp) {
        return ActionStateComp->GetState() == EActionState::Planting
			|| ActionStateComp->GetState() == EActionState::Defusing;
    }
    return false;
}   

bool ABaseCharacter::CanSeeThisActor(const APawn* Target) const
{
    float FOVDegrees = 80.f;
    float MaxDistance = 0.f;
    if (!IsValid(Target)) return false;

    const UWorld* World = GetWorld();
    if (!World) return false;

    // ----- View origin + direction -----
    FVector ViewLoc;
    FRotator ViewRot;

    // Prefer controller view when available (more accurate for players), otherwise fall back to actor view.
    if (const AController* C = GetController())
    {
        C->GetPlayerViewPoint(ViewLoc, ViewRot);
    }
    else
    {
        ViewLoc = GetPawnViewLocation();
        ViewRot = GetViewRotation();
    }

    // Aim at target "eyes" if possible; otherwise use actor location.
    FVector TargetLoc = Target->GetActorLocation();
    if (const ACharacter* TargetChar = Cast<ACharacter>(Target))
    {
        TargetLoc = TargetChar->GetPawnViewLocation();
    }

    const FVector ToTarget = TargetLoc - ViewLoc;
    const float DistSq = ToTarget.SizeSquared();

    if (MaxDistance > 0.f && DistSq > FMath::Square(MaxDistance))
        return false;

    // ----- Field of view check -----
    const FVector ViewForward = ViewRot.Vector();
    const FVector ToTargetDir = ToTarget.GetSafeNormal();

    const float CosHalfFOV = FMath::Cos(FMath::DegreesToRadians(FOVDegrees * 0.5f));
    const float CosAngle = FVector::DotProduct(ViewForward, ToTargetDir);

    if (CosAngle < CosHalfFOV)
        return false;

    // ----- Line of sight check -----
    FCollisionQueryParams Params(SCENE_QUERY_STAT(CharViewLOS), /*bTraceComplex=*/true);
    Params.AddIgnoredActor(this);

    // Ignore anything attached to us (weapons, components that might block the trace).
    TArray<AActor*> AttachedActors;
    GetAttachedActors(AttachedActors);
    for (AActor* A : AttachedActors)
    {
        Params.AddIgnoredActor(A);
    }

    FHitResult Hit;
    const bool bHit = World->LineTraceSingleByChannel(Hit, ViewLoc, TargetLoc, ECC_Visibility, Params);

    if (!bHit)
        return true; // Nothing blocked the trace.

    AActor* HitActor = Hit.GetActor();
    return (HitActor == Target) || (HitActor && HitActor->IsOwnedBy(Target));
}

static FVector GetBoneOrSocketLoc(const USkeletalMeshComponent* Mesh, const FName& Name)
{
    if (!Mesh) return FVector::ZeroVector;
    if (Mesh->DoesSocketExist(Name)) return Mesh->GetSocketLocation(Name);
    return Mesh->GetBoneLocation(Name);
}

FVector ABaseCharacter::GetAimPoint(EAimPointPolicy Policy, float HeadChance01) const
{
    return GetAimPointInternal(Policy, HeadChance01);
}

FVector ABaseCharacter::GetAimPointInternal(EAimPointPolicy Policy, float HeadChance01) const
{
    const USkeletalMeshComponent* SkelMeshComp = GetMesh();
    const FVector Fallback = GetActorLocation();

    if (!SkelMeshComp) return Fallback;

    static const FName Head(TEXT("head"));
    static const FName Chest(TEXT("spine_03"));
    static const FName Pelvis(TEXT("pelvis"));

    auto HeadLoc = [&]() { return GetBoneOrSocketLoc(SkelMeshComp, Head);   };
    auto ChestLoc = [&]() { return GetBoneOrSocketLoc(SkelMeshComp, Chest);  };
    auto PelvisLoc = [&]() { return GetBoneOrSocketLoc(SkelMeshComp, Pelvis); };

    switch (Policy)
    {
    case EAimPointPolicy::Center:     return Fallback;
    case EAimPointPolicy::Head:       return HeadLoc();
    case EAimPointPolicy::Chest:      return ChestLoc();
    case EAimPointPolicy::Pelvis:     return PelvisLoc();
    case EAimPointPolicy::RandomBody:
    {
        const int32 Pick = FMath::RandRange(0, 2);
        return (Pick == 0) ? ChestLoc() : (Pick == 1) ? PelvisLoc() : HeadLoc();
    }
    case EAimPointPolicy::HeadOrBody:
    default:
    {
        const float R = FMath::FRand();
        return (R < HeadChance01) ? HeadLoc() : ChestLoc();
    }
    }
}

void ABaseCharacter::GetActorEyesViewPoint(FVector& out_Location, FRotator& out_Rotation) const
{
    out_Location = FpsPivot->GetComponentLocation();
	out_Rotation = GetViewRotation();
}

float ABaseCharacter::GetSpineKickAlpha() const
{
	return SpineKickAlpha;
}

void ABaseCharacter::PlayEffectHitReact() {
    UE_LOG(LogTemp, Warning, TEXT("PlayEffectHitReact"));
    if (SpineKickTimeline.IsPlaying())
    {
        return;
    }

    // if is firing then return
    if (ActionStateComp) {
        if (ActionStateComp->IsInState(EActionState::Firing)) {
            return;
        }
    }

    SpineKickTimeline.PlayFromStart();
}

void ABaseCharacter::OnSpineKickUpdate(float Value)
{
    SpineKickAlpha = Value;

    float CamKickRawDeg = 1.0f;
    float CamKickRollDeg = 0.5f;

    FRotator CamFpsBaseRelRot = FRotator::ZeroRotator;
    if (CameraFps)
    {
       /* const float Raw = CamKickRawDeg * Value;
        const float Roll = CamKickRollDeg * Value;

        CameraFps->SetRelativeRotation(
            CamFpsBaseRelRot + FRotator(0, Raw, Roll)
        );*/
    }
}

void ABaseCharacter::SetupSpineKickTimeline()
{
    if (!CachedCharacterAsset || !CachedCharacterAsset->SpineKickCurve) return;

    FOnTimelineFloat Update;
    Update.BindUFunction(this, FName("OnSpineKickUpdate"));
    SpineKickTimeline.AddInterpFloat(CachedCharacterAsset->SpineKickCurve, Update);
    SpineKickTimeline.SetLooping(false);
}

void ABaseCharacter::OnRep_CharacterSkin() {
	ApplyVisualByRole(GetCharacterRole());

    UE_LOG(LogTemp, Warning, TEXT("fbuggSetting onrepcharacter skin for spawned pawn %d"), CharacterSkin);

}

void ABaseCharacter::SetCharacterSkin(int32 NewSkin) {
    if (CharacterSkin == NewSkin) {
        return;
    }
    CharacterSkin = NewSkin;
    OnRep_CharacterSkin();
}

void ABaseCharacter::OnRep_IsPermanentDead() {
    if (!IsPermanentDead())
        return;
    UCapsuleComponent* Capsule = GetCapsuleComponent();
    USkeletalMeshComponent* SkelMesh = GetMesh();
    if (!Capsule || !SkelMesh)
        return;

    Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SkelMesh->SetCollisionProfileName(TEXT("Ragdoll"));
    SkelMesh->SetSimulatePhysics(true);
}

void ABaseCharacter::DisableDeadMeshTick()
{
    //USkeletalMeshComponent* SkelMesh = GetMesh();
    //if (!SkelMesh) return;

    //// Stop anim evaluation to save CPU
    //SkelMesh->bPauseAnims = true;
    //SkelMesh->SetComponentTickEnabled(false);

    //// Disable expensive always-tick mode
    //SkelMesh->VisibilityBasedAnimTickOption =
    //    EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
}

void ABaseCharacter::UpdateInteractComponentTick()
{
	UE_LOG(LogTemp, Warning, TEXT("UpdateInteractComponentTick called. IsLocallyControlled: %s"), IsLocallyControlled() ? TEXT("true") : TEXT("false"));
    if (InteractComp)
    {
        InteractComp->SetComponentTickEnabled(IsLocallyControlled());
    }
}

void ABaseCharacter::ShowNameText(bool bShow)
{
    if (NameText)
    {
        const FString Name = GetPlayerName();
        NameText->SetText(FText::FromString(Name));
        NameText->SetVisibility(bShow, true);
    }
}