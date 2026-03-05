#include "Game/Characters/BaseCharacter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Game/Framework/ShooterGameMode.h"
#include "Game/Projectiles/ThrownProjectile.h"
#include "Game/Framework/MyPlayerController.h"
#include "Engine/DamageEvents.h"
#include "Game/Utils/Damage/MyPointDamageEvent.h"
#include "Game/Framework/MyPlayerState.h"
#include "Game/Framework/ShooterGameState.h"
#include "Game/AI/BotAIController.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "Game/Characters/Components/InteractComponent.h"
#include "Game/Characters/Components/HealthComponent.h"     
#include "NiagaraFunctionLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
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
#include "Shared/Data/GlobalDataAsset.h"
#include "Shared/Data/Items/FirearmConfig.h"
#include "Game/Characters/Components/ItemUseComponent.h"
#include "Game/Modes/Zombie/ZombieMode.h"
#include "Game/Modes/Spike/Spike.h"
#include "Components/TextRenderComponent.h"
#include "Game/Subsystems/ActorManager.h"
#include "Shared/Data/Items/ThrowableConfig.h"

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
    StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));

    // Components
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

    // Initialize dependencies for components
    EquipComp->Init();
	ActionStateComp->Init();
	AnimationComp->Init();
	InteractComp->Init();
	InventoryComp->Init();
	ItemUseComp->Init();
	ItemVisualComp->Init();
	LagCompensationComp->Init();    
	PickupComponent->Init();
	SpikeComp->Init();
	ThrowableComp->Init();
	WeaponFireComp->Init();
	WeaponMeleeComp->Init();

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->NavAgentProps.bCanCrouch = true;
    MoveComp->MaxWalkSpeedCrouched = CROUCH_WALK_SPEED;

    GetMesh()->SetRelativeLocation(TPSMeshRelLoc);
    GetMesh()->SetRelativeRotation(TPSMeshRelRot);

    // set same eye position
    BaseEyeHeight = 70;
    CrouchedEyeHeight = 70;
	FpsPivot->SetRelativeLocation(FVector(0.f, 0.f, -TPSMeshRelLoc.Z + BaseEyeHeight));

    MeshFps->SetRelativeLocation(FVector(0.f, 0.f, -170.f));
    MeshFps->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

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
    CachedCharacterAsset = GameManager->CharacterAsset.Get();
	check(CachedCharacterAsset);

	AActorManager* ActorMgr = AActorManager::Get(GetWorld());
    if (ActorMgr) {
        ActorMgr->RegisterPlayer(this);
    }

	// setup, delegate bindings, etc.
	BindDelegates();
    BindMontageNotifies();

    SetupCrouchTimeline();
    SetupStunTimeline();
    SetupSpineKickTimeline();
    SetupPerception();
    SetupFlashPostProcess();
    SetupInitialInventory();

    BasePivotFpsZ = FpsPivot->GetRelativeLocation().Z;

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

    if (AActorManager* AM = AActorManager::Get(GetWorld()))
    {
        AM->UnregisterPlayer(this);
    }
    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void ABaseCharacter::BindDelegates() {
    EquipComp->OnActiveItemChanged.AddUObject(
        this,
        &ABaseCharacter::UpdateCurrentWeapon
    );
    EquipComp->OnActiveItemChanged.AddUObject(
        WeaponMeleeComp,
        &UWeaponMeleeComponent::HandleActiveItemChanged
    );  
    EquipComp->OnActiveItemChanged.AddUObject(
        WeaponFireComp,
        &UWeaponFireComponent::OnActiveItemChanged
    );
    EquipComp->OnActiveItemChanged.Broadcast(EquipComp->GetActiveItemId());
    HealthComp->OnDeath.AddUObject(this, &ABaseCharacter::OnHealthDepleted);
    RoleComp->OnRoleChanged.AddUObject(this, &ABaseCharacter::HandleRoleChanged);
    this->HandleRoleChanged(RoleComp->GetRole(), RoleComp->GetRole());
}

void ABaseCharacter::UpdateCurrentWeapon(EItemId NewWeaponId)
{
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

    if (CameraComp->IsFPS()) // only for fps to save performance
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

#if !UE_SERVER // to save performance for dedicated server
    // logic sound
    UpdateFootstepSound(DeltaTime);
    if (NameText->IsVisible()) {
        UpdateNameTextRotation();
    }
#endif
}

void ABaseCharacter::UpdateNameTextRotation()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->PlayerCameraManager) {
        return;
    }

    const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
    const FVector TextLoc = NameText->GetComponentLocation();

    const FRotator LookAt = (CamLoc - TextLoc).Rotation();
    NameText->SetWorldRotation(FRotator(0.f, LookAt.Yaw, 0.f));
}

void ABaseCharacter::UpdateFootstepSound(float DeltaTime) {
	UWorld* World = GetWorld();

    const float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - LastFootstepTime >= FootstepInterval)
    {
        if (!CanPlayFootstep()) {
            return;
        }
        const float Speed = GetVelocity().Size2D();

        if (Speed > 500.f) {
            FootstepInterval = 0.25f;
        }
        else if (Speed > 410.f) {
            FootstepInterval = 0.3f;
        }
        else {
            FootstepInterval = 0.35f;
		}
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
    CameraComp->ToggleView();
}

USkeletalMeshComponent* ABaseCharacter::GetCurrentMesh() const
{
    if (CameraComp->IsFPS())
    {
        return MeshFps;
    }
   
    return GetMesh();
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
    if (!WeaponFireComp->CanWeaponAim()) {
        return;
    }
    if (bIsAiming) {
        return;
    }

    if (ActionStateComp->IsInState(EActionState::Reloading)) {
        return; // can't aim while reloading
	}
    
	// if is locally controlled, update immediately
    if (IsLocallyControlled()) {
		// Predictive update
		bIsAiming = true;
        CameraComp->SetAiming(true);
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
        CameraComp->SetAiming(false);
        UpdateMaxWalkSpeed();
    }
    ServerSetAiming(false);
}


void ABaseCharacter::OnRep_IsAiming()
{
    CameraComp->SetAiming(bIsAiming);
}

void ABaseCharacter::ServerSetAiming_Implementation(bool bNewAiming)
{
    if (!IsAlive()) {
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

void ABaseCharacter::UpdateMaxWalkSpeed() {
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
            // Speed *= 0.2f; // aiming penalty
        }

        if (RoleComp->GetRole() == ECharacterRole::Zombie) {
            Speed += 10.f; // zombie role moves faster
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
    if (HealthComp->IsDead())
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

    LastHitByController = EventInstigator;
	bLastHitWasHeadshot = false;
    if (EventInstigator)
    {
        DamageInstigators.AddUnique(EventInstigator);
    }

	UItemsManager* ItemsMgr = UItemsManager::Get(GetWorld());
    if (DamageEvent.IsOfType(FMyPointDamageEvent::ClassID))
    {
        const FMyPointDamageEvent* MyEvent =
            static_cast<const FMyPointDamageEvent*>(&DamageEvent);

        EItemId WeaponId = static_cast<EItemId>(MyEvent->WeaponID);
		LastDamageCauser = ItemsMgr->GetItemById(WeaponId);
		bLastHitWasHeadshot = MyEvent->bIsHeadshot;
    }
    else if (DamageCauser) {
        if (const AThrownProjectile* Projectile = Cast<AThrownProjectile>(DamageCauser))
        {
            LastDamageCauser = Projectile->GetWeaponData();
        }
        else if (DamageCauser->IsA(ASpike::StaticClass()))
        {
            LastDamageCauser = ItemsMgr->GetItemById(EItemId::SPIKE);
        }
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
    if (NotifyName == "MeleeAttack")
    {
        OnMeleeNotify();
    }
    else if (NotifyName == "Notify_GrabMag")
    {
        ItemVisualComp->OnNotifyGrabMag();
	}
    else if (NotifyName == "Notify_InsertMag")
    {
        ItemVisualComp->OnNotifyInsertMag();
	}
    else if (NotifyName == "Throw_NadeRelease")
    {
        ThrowableComp->OnNadeRelease();
	}
}

// When health reaches zero, soldier -> become zombie
void ABaseCharacter::OnHealthDepleted()
{
    // Play animation, ragdoll, notify game mode, etc.
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
            CameraComp->SetAiming(false);
		}
    }
}

// server function to apply death effects
void ABaseCharacter::ApplyRealDeath(bool bDropInventory, bool bInPermanentDead)
{
    if (SpikeComp->IsEnabled()) {
		SpikeComp->OnOwnerDead();
    }

    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();
    ActionStateComp->ForceSetState(EActionState::Disabled);

    if (ABotAIController* AI = Cast<ABotAIController>(GetController()))
    {
		AI->StopLogic("OwnerDead");
    }
    if (bDropInventory)
    {
        InventoryComp->DropAllItems();
    }
    EquipComp->UnequipCurrentItem();
    MulticastCharacterDeath(); 
    DisableDeadMeshTick();
    if (bIsPermanentDead != bInPermanentDead) {
        bIsPermanentDead = bInPermanentDead;
        OnRep_IsPermanentDead();
    }
}

void ABaseCharacter::MulticastCharacterDeath_Implementation()
{
    NameText->SetVisibility(false);

    UCapsuleComponent* Capsule = GetCapsuleComponent();
    USkeletalMeshComponent* SkelMesh = GetMesh();
    Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SkelMesh->SetCollisionProfileName(TEXT("Ragdoll"));
    SkelMesh->SetSimulatePhysics(true);

    // if is hero, play sound hero dead
    if (GetCharacterRole() == ECharacterRole::Hero) {
        AudioComp->PlayHeroDeath();
    }
    else if (GetCharacterRole() == ECharacterRole::Zombie) { 
        AudioComp->PlayZombieDeath();
	}
    else {
        AudioComp->PlaySoldierDeath();
    }
    AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
    if (!MyPC) {
        return;
	}

	bool bIsLocalPlayer = MyPC->GetViewTarget() == this;
	auto LocalPC = Cast<AMyPlayerController>(GetController());
    if (LocalPC) {
        float Now = 0;
        if (const UWorld* World = GetWorld())
        {
            if (const AGameStateBase* GS = World->GetGameState())
            {
                Now = GS->GetServerWorldTimeSeconds();
            }
        }
        LocalPC->SetTimeOfDeath(Now);
    }
    if (bIsLocalPlayer) {
        MyPC->SetIgnoreLookInput(true);
        MyPC->SetIgnoreMoveInput(true);
        
        if (CameraComp->IsFPS()) {
            ChangeView(); // switch to tps view
        }

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
	OnHit.Broadcast();
}

void ABaseCharacter::PlayBloodFx(const FVector& HitLocation, const FVector& HitNormal)
{
    PlayEffectHitReact();
    if (IsFpsViewMode()) {
		return; // no blood fx in fps mode
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
}

void ABaseCharacter::PlayStunEffect(const float& Strength) 
{
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
    return CameraComp->GetAimSensitivity();
}

void ABaseCharacter::OnPlantSpikeStarted() {
    AudioComp->PlayPlantSpike();
}

void ABaseCharacter::OnPlantSpikeStopped() {
    AudioComp->StopPlantSpike();
}

void ABaseCharacter::OnDefuseSpikeStarted() {
    AudioComp->PlayDefuseSpike();
}

void ABaseCharacter::OnDefuseSpikeStopped() {
    AudioComp->StopDefuseSpike();
}

void ABaseCharacter::Destroyed()
{
    Super::Destroyed();
    ItemVisualComp->OnOwnerDead();
}

void ABaseCharacter::ServerSetIsSlow_Implementation(bool bNewIsSlow)
{
    if (!IsAlive())
    {
        return;
    }
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
    return !HealthComp->IsDead();
}

bool ABaseCharacter::IsDead() const
{
    return HealthComp->IsDead();
}

void ABaseCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);
    PlayLandingSound();
}

void ABaseCharacter::PlayLandingSound()
{
    AudioComp->PlayLanding();
}

void ABaseCharacter::BecomeViewTarget(APlayerController* PC)
{
    Super::BecomeViewTarget(PC);
    if (!PC->IsLocalController()) {
        return;
    }

    AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC);
    if (!MyPC)
    {
        return;
    }
    MyPC->OnBecomeViewTarget(this);
    CameraComp->OnBecomeViewTarget(MyPC);
}

void ABaseCharacter::EndViewTarget(APlayerController* PC)
{
    Super::EndViewTarget(PC);

    if (!IsLocallyControlled() && PC && PC->IsLocalController())
    {
		//SetFpsView(false);
    }
    CameraComp->OnEndViewTarget(PC);
}

FVector ABaseCharacter::GetThrowableLocation() const
{
    FVector EyeLoc;
    FRotator EyeRot;
    GetActorEyesViewPoint(EyeLoc, EyeRot);

    return EyeLoc
        + GetActorForwardVector() * 20.f
        + FVector(0.f, 0.f, 20.f);
}

EMovementState ABaseCharacter::GetCurrentMovementState() const {
    return CurrentMovementState;
}

bool ABaseCharacter::IsFpsViewMode() const {
    return CameraComp->IsFPS();
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
    return EquipComp->GetEquippedAnimState();
}

UThrowableComponent* ABaseCharacter::GetThrowableComponent() const {
    return ThrowableComp.Get();
}

USpikeComponent* ABaseCharacter::GetSpikeComponent() const {
    return SpikeComp.Get();
}

// this should happens on server only
void ABaseCharacter::ApplyDefaultsForRole(ECharacterRole NewRole) {
    ActionStateComp->ForceSetState(EActionState::Idle);

    if (NewRole == ECharacterRole::Zombie)
    {
        HealthComp->SetMaxHealth(FGameConstants::INIT_HEALTH_ZOMBIE);
        HealthComp->ResetHealth();

        InventoryComp->OnBecomeZombie();
        EquipComp->SelectSlot(FGameConstants::SLOT_MELEE);
    }
    else if (NewRole == ECharacterRole::Hero)
    {

        HealthComp->SetMaxHealth(FGameConstants::INIT_HEALTH_HERO);
        HealthComp->ResetHealth();

        InventoryComp->OnBecomeHero();
        EquipComp->SelectSlot(FGameConstants::SLOT_MELEE);
    }
    else // human
    {
        HealthComp->SetMaxHealth(FGameConstants::INIT_HEALTH_SOLIDER);
        HealthComp->ResetHealth();
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
        ApplyDefaultsForRole(NewRole);
    }

    // play effect if human to zombie
    if (NewRole == ECharacterRole::Zombie)
    {
        PlayZombieSpawnEffects();
    }
    else if (NewRole == ECharacterRole::Hero) {
        AudioComp->PlayHeroSpawn();
    }

	// role changed mean update walk speed
	UpdateMaxWalkSpeed();
}

void ABaseCharacter::BindMontageNotifies() {
    if (UAnimInstance* FPSAnim = MeshFps->GetAnimInstance())
    {
        FPSAnim->OnPlayMontageNotifyBegin.AddDynamic(this, &ABaseCharacter::OnNotifyBegin);
    }
  
    if (UAnimInstance* TPSAnim = GetMesh()->GetAnimInstance())
    {
        TPSAnim->OnPlayMontageNotifyBegin.AddDynamic(this, &ABaseCharacter::OnNotifyBegin);
    }
}

void ABaseCharacter::ApplyVisualByRole(ECharacterRole NewRole)
{
    if (!CachedCharacterAsset) {
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
        else if (CharacterSkin == FGameConstants::SKIN_CHARACTER_YIN) {
            VisualSet = CachedCharacterAsset->YinVisualSet;
		}
    }

    if (!VisualSet) {
        return;
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
    BindMontageNotifies();

    // -------- Visual reset --------
    if (NewRole == ECharacterRole::Zombie)
    {
        ItemVisualComp->OnOwnerDead();
    }
}

void ABaseCharacter::ApplyLoadoutByRole(ECharacterRole NewRole)
{
    if (NewRole == ECharacterRole::Zombie)
    {
        WeaponFireComp->SetEnabled(false);
        ThrowableComp->SetEnabled(false);
        PickupComponent->SetEnabled(false);
    }
    else if (NewRole == ECharacterRole::Hero)
    {
        WeaponFireComp->SetEnabled(false);
        PickupComponent->SetEnabled(false);
        ThrowableComp->SetEnabled(true);
    }
    else
    {
        // human
    }
}


bool ABaseCharacter::IsCharacterRole(ECharacterRole InRole) const
{
    return RoleComp->GetRole() == InRole;
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
    ItemUseComp->PrimaryPressed();
}

void ABaseCharacter::RequestPrimaryActionReleased() {
    if (!CanAct()) {
        return;
    }
    ItemUseComp->PrimaryReleased();
}

void ABaseCharacter::RequestSecondaryActionPressed() {
    if (!CanAct()) {
        return;
    }
    ItemUseComp->SecondaryPressed();
}

void ABaseCharacter::RequestSecondaryActionReleased() {
    if (!CanAct()) {
        return;
    }
    ItemUseComp->SecondaryReleased();
}

void ABaseCharacter::RequestReloadPressed() {
    if (!CanAct()) {
        return;
    }
    ItemUseComp->ReloadPressed();
}

bool ABaseCharacter::CanAct() {
    if (!IsAlive()) {
        return false;
    }
    return true;
}

void ABaseCharacter::SetupCrouchTimeline()
{
    if (!CachedCharacterAsset || !CachedCharacterAsset->CrouchCurve) {
        return;
    }

    FOnTimelineFloat Update;
    Update.BindUFunction(this, FName("HandleCrouchProgress"));
    CrouchTimeline.AddInterpFloat(CachedCharacterAsset->CrouchCurve, Update);
    CrouchTimeline.SetLooping(false);
}

void ABaseCharacter::SetupStunTimeline()
{
    if (!CachedCharacterAsset || !CachedCharacterAsset->StunCurve) {
        return;
    }

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
    StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
    StimuliSource->RegisterWithPerceptionSystem();
}

void ABaseCharacter::SetupFlashPostProcess()
{
    if (!CachedCharacterAsset || !CachedCharacterAsset->FlashPPMat) {
        return;
    }

    FlashMID = UMaterialInstanceDynamic::Create(CachedCharacterAsset->FlashPPMat, this);
    CameraFps->PostProcessSettings.AddBlendable(FlashMID, 1.0f); 
    CameraTps->PostProcessSettings.AddBlendable(FlashMID, 1.0f);
    FlashMID->SetScalarParameterValue(TEXT("Intensity"), 0.f);
}

bool ABaseCharacter::IsSpikeMode() const
{
    const AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>();
    return GS && (GS->GetMatchMode() == EMatchMode::Spike);
}


ECharacterRole ABaseCharacter::GetCharacterRole() const {
    return RoleComp->GetRole();
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

    InventoryComp->InitBasicWeapon();
    EquipComp->AutoSelectBestWeapon();
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
    MoveComp->SetMovementMode(MOVE_Walking);
	UpdateMaxWalkSpeed();
}

void ABaseCharacter::Revive()
{
    if (!HasAuthority()) return;

    MulticastRevive();
    HealthComp->SetHealth(HealthComp->GetMaxHealth());
	if (ABotAIController* AI = Cast<ABotAIController>(GetController())) {
        AI->StartLogic();
    }

    ActionStateComp->ForceSetState(EActionState::Idle);
    EquipComp->AutoSelectBestWeapon();
}

void ABaseCharacter::PlayZombieSpawnEffects() {
    AudioComp->PlayZombieSpawn();
    if (!CachedCharacterAsset || !CachedCharacterAsset->TurnToZombieFx)
    {
        return;
    }

    FVector SpawnLocation = FVector::ZeroVector;
    SpawnLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
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

bool ABaseCharacter::IsHero() const {
    return GetCharacterRole() == ECharacterRole::Hero;
}

bool ABaseCharacter::IsZombie() const {
    return GetCharacterRole() == ECharacterRole::Zombie;
}

bool ABaseCharacter::IsPermanentDead() const {
    return bIsPermanentDead;
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
    return ActionStateComp->GetState() == EActionState::Planting
        || ActionStateComp->GetState() == EActionState::Defusing;
}

bool ABaseCharacter::CanSeeThisActor(const APawn* Target) const
{
    float FOVDegrees = 80.f;
    float MaxDistance = 0.f;
    if (!IsValid(Target)) return false;

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
    const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, ViewLoc, TargetLoc, ECC_Visibility, Params);
    if (!bHit)
    {
        return true; // Nothing blocked the trace.
    }

    AActor* HitActor = Hit.GetActor();
    return (HitActor == Target) || (HitActor && HitActor->IsOwnedBy(Target));
}

static FVector GetBoneOrSocketLoc(const USkeletalMeshComponent* Mesh, const FName& Name)
{
    if (!Mesh) {
        return FVector::ZeroVector;
    }
    if (Mesh->DoesSocketExist(Name)) {
        return Mesh->GetSocketLocation(Name);
    }
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
    if (SpineKickTimeline.IsPlaying())
    {
        return;
    }

    // if is firing then return
    if (ActionStateComp->IsInState(EActionState::Firing)) {
        return;
    }
    SpineKickTimeline.PlayFromStart();
}

void ABaseCharacter::OnSpineKickUpdate(float Value)
{
    SpineKickAlpha = Value;
}

void ABaseCharacter::SetupSpineKickTimeline()
{
    if (!CachedCharacterAsset || !CachedCharacterAsset->SpineKickCurve) {
        return;
    }

    FOnTimelineFloat Update;
    Update.BindUFunction(this, FName("OnSpineKickUpdate"));
    SpineKickTimeline.AddInterpFloat(CachedCharacterAsset->SpineKickCurve, Update);
    SpineKickTimeline.SetLooping(false);
}

void ABaseCharacter::OnRep_CharacterSkin() {
	ApplyVisualByRole(GetCharacterRole());
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

void ABaseCharacter::ShowNameText(bool bShow)
{
    NameText->SetVisibility(bShow, true);

    if (bShow)
    {
        NameText->SetText(FText::FromString(GetPlayerName()));
	}
}

bool ABaseCharacter::Heal(float HealAmount)
{
    const bool bHealed = HealthComp->ApplyHeal(HealAmount);
    if (!bHealed) return false;

    // Play effect heal (feedback belongs to character)
    if (CachedCharacterAsset && CachedCharacterAsset->HealFx)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            CachedCharacterAsset->HealFx,
            GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            true
        );
    }
    AudioComp->PlayHeal();
    return true;
}