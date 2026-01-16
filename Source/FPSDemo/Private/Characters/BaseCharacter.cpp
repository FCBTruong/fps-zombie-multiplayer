#include "Characters/BaseCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Game/ShooterGameMode.h"
#include "Projectiles/ThrownProjectile.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Controllers/MyPlayerController.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Damage/MyDamageType.h"
#include "Engine/DamageEvents.h"
#include "Damage/MyPointDamageEvent.h"
#include "Controllers/MyPlayerState.h"
#include "Game/ShooterGameState.h"
#include "Perception/AISense_Damage.h"
#include "Controllers/BotAIController.h"
#include "Weapons/WeaponState.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/InteractComponent.h"
#include "Components/HealthComponent.h"     
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
#include "Components/PickupComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/AnimationComponent.h"
#include "Components/CharAudioComponent.h"
#include "Components/CharCameraComponent.h"
#include "Components/EquipComponent.h"
#include "Components/ActionStateComponent.h"
#include "Components/ItemVisualComponent.h"
#include "Components/WeaponFireComponent.h"
#include "Components/WeaponMeleeComponent.h"
#include "Components/ThrowableComponent.h"
#include "Components/SpikeComponent.h"
#include "Components/RoleComponent.h"
#include "Game/ItemsManager.h"
#include "Asset/CharacterAsset.h"
#include "Materials/MaterialInterface.h"
#include "Game/GlobalDataAsset.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Items/ThrowableConfig.h"
#include "Components/RoleComponent.h"
#include "Items/FirearmConfig.h"
#include "Components/ItemUseComponent.h"
#include "Game/ZombieMode.h"

const FVector ABaseCharacter::TPSMeshRelLoc(0.f, 0.f, -88.f);
const FRotator ABaseCharacter::TPSMeshRelRot(0.f, -90.f, 0.f);

// Sets default values
ABaseCharacter::ABaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

    FpsPivot = CreateDefaultSubobject<USceneComponent>(TEXT("FpsPivot"));
    FpsPivot->SetupAttachment(GetRootComponent());

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBloom"));
    CameraBoom->SetupAttachment(RootComponent);

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
        BaseEyeHeight = 60;
        CrouchedEyeHeight = 60;
		FpsPivot->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight));
    }
    if (MeshFps)
    {
        MeshFps->SetRelativeLocation(FVector(0.f, 0.f, -170.f));
        MeshFps->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    }

    FlashPP = CreateDefaultSubobject<UPostProcessComponent>(TEXT("FlashPP"));
    FlashPP->SetupAttachment(CameraFps);
    FlashPP->bUnbound = false;
    FlashPP->BlendWeight = 1.0f;
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
    CachedCharacterAsset = GameManager->CharacterAsset.Get();
    GameManager->RegisterPlayer(this);

	// setup, delegate bindings, etc.
	BindDelegates();
    SetupCrouchTimeline();
    SetupStunTimeline();
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
        HealthComp->OnDeath.AddUObject(this, &ABaseCharacter::HandleDeath);
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

void ABaseCharacter::ApplyTeamMesh() // TODO later, for spike mode
{
    UE_LOG(LogTemp, Warning, TEXT("Setting mesh based on team"));
    if (!CachedCharacterAsset) {
        return;
    }
    // get player state
    AMyPlayerState* MyPS = Cast<AMyPlayerState>(GetPlayerState());
    AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>();
    if (!GS) {
        UE_LOG(LogTemp, Warning, TEXT("GS is null in SetMeshBaseOnTeam"));
        return;
    }
    if (MyPS) {
        USkeletalMesh* NewMesh = nullptr;

        if (MyPS->GetTeamId() == ETeamId::Attacker) {
            NewMesh = CachedCharacterAsset->TerroristMesh;
        }
        else {
            NewMesh = CachedCharacterAsset->CounterTerroristMesh;
        }
        if (NewMesh)
        {
            UE_LOG(LogTemp, Warning, TEXT("Setting new mesh for team"));
            GetMesh()->SetSkeletalMesh(NewMesh);
        }
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("MyPS is null in SetMeshBaseOnTeam"));
    } 
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

        const FRotator TargetPivotRot(AimRot.Pitch, 0.f, 0.f);

        const FRotator Smoothed =
            FMath::RInterpTo(FpsPivot->GetRelativeRotation(), TargetPivotRot, DeltaTime, 20.f);

        FpsPivot->SetRelativeRotation(Smoothed);
    }
    // logic sound
    UpdateFootstepSound(DeltaTime);
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
    
    // log data
    UE_LOG(LogTemp, Warning,
        TEXT("OnStartCrouch: HalfHeightAdjust = %f, ScaledHalfHeightAdjust = %f"),
        HalfHeightAdjust,
        ScaledHalfHeightAdjust
	);

    // Cancel the instant camera drop caused by capsule shrinking
    CurrentCrouchCompZ += HalfHeightAdjust;

    CrouchFromZ = CurrentCrouchCompZ;
    CrouchToZ = 0.f;

    // Snap to the compensated position (no pop), then smooth toward target
    FVector Loc = FpsPivot->GetRelativeLocation();
    Loc.Z = BasePivotFpsZ + CurrentCrouchCompZ;
    FpsPivot->SetRelativeLocation(Loc);

    CrouchTimeline.PlayFromStart();

    UpdateMaxWalkSpeed();
}

void ABaseCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    // Cancel the instant camera rise caused by capsule expanding
    CurrentCrouchCompZ -= HalfHeightAdjust;

    CrouchFromZ = CurrentCrouchCompZ;
    CrouchToZ = 0.f;

    FVector Loc = FpsPivot->GetRelativeLocation();
    Loc.Z = BasePivotFpsZ + CurrentCrouchCompZ;
    FpsPivot->SetRelativeLocation(Loc);

    CrouchTimeline.PlayFromStart(); // forward again (not Reverse)
    UpdateMaxWalkSpeed();
}

void ABaseCharacter::HandleCrouchProgress(float Alpha)
{
    CurrentCrouchCompZ = FMath::Lerp(CrouchFromZ, CrouchToZ, Alpha);

    FVector Loc = FpsPivot->GetRelativeLocation();
    Loc.Z = BasePivotFpsZ + CurrentCrouchCompZ;
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

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ABaseCharacter, bIsAiming);
	DOREPLIFETIME(ABaseCharacter, SpeedMultiplier);
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
    if (IsLocallyControlled()) {
		return; // already handled locally
	}
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

	// Weight penalty
	float Weight = Config->Weight;
    Weight = FMath::Max(Weight, 0.01f);
	Ratio = Ratio / Weight;

	UE_LOG(LogTemp, Warning, TEXT("GetSpeedWalkRatio: Weight = %f, Ratio = %f"), Weight, Ratio);

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
        Speed = NORMAL_WALK_SPEED * GetSpeedWalkRatio();
        if (RoleComp) {
            if (RoleComp->GetRole() == ECharacterRole::Zombie) {
                Speed *= 1.1f; // zombie role moves faster
			}
        }
    }
	Speed *= SpeedMultiplier;
    Speed *= 1.2; // for zombie mode TODO: check later
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

	UE_LOG(LogTemp, Warning, TEXT("ABaseCharacter::TakeDamage called with DamageAmount: %f"), DamageAmount);
    if (HealthComp && HealthComp->IsDead())
    {
        return 0.f; // already dead
	}
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    
    /* const FArmorState* Armor = InventoryComp->GetArmorState();
    if (Armor)
    {
        if (Armor->ArmorPoints > 0)
        {
            float DamageToHealth = ActualDamage * Armor->ArmorRatio;
            float DamageToArmor = (ActualDamage - DamageToHealth) * Armor->ArmorEfficiency;

            if (DamageToArmor >= Armor->ArmorPoints)
            {
                float Overflow = DamageToArmor - Armor->ArmorPoints;
                DamageToHealth += Overflow;
                Armor->ArmorPoints = 0.0f;
            }
            else
            {
                Armor->ArmorPoints -= DamageToArmor;
            }

            ActualDamage = DamageToHealth;
		}
	}*/
	UE_LOG(LogTemp, Warning, TEXT("ABaseCharacter::TakeDamage called with DamageAmount: %f"), DamageAmount);
    LastHitByController = EventInstigator;
	bLastHitWasHeadshot = false;

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
		AThrownProjectile* Projectile = Cast<AThrownProjectile>(DamageCauser);
        if (Projectile) {
			LastDamageCauser = Projectile->GetWeaponData();
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
            // AICon->SetFocus(EventInstigator);
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

// This function called on server when health reaches zero
void ABaseCharacter::HandleDeath()
{
    // Play animation, ragdoll, notify game mode, etc.
	UE_LOG(LogTemp, Warning, TEXT("Character has died."));
    if (HasAuthority())
    {
        AShooterGameMode* GM = Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this));
        if (!GM) {
            return;
        }
        GM->OnCharacterKilled(LastHitByController.Get(), this, LastDamageCauser.Get(), bLastHitWasHeadshot);
        LastHitByController = nullptr; // reset after use
        LastDamageCauser = nullptr; // reset after use

        if (bIsAiming) {
            bIsAiming = false;
            if (CameraComp) CameraComp->SetAiming(false);
		}
    }
}

void ABaseCharacter::ApplyRealDeath(bool bDropInventory)
{
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();

    if (AAIController* AI = Cast<AAIController>(GetController()))
        if (UBrainComponent* Brain = AI->GetBrainComponent())
            Brain->StopLogic(TEXT("Bot died"));

    if (bDropInventory && InventoryComp)
        InventoryComp->DropAllItems();

    MulticastCharacterDeath(); // NetMulticast trong Character
}

void ABaseCharacter::MulticastCharacterDeath_Implementation()
{
    // hide visual
    if (ItemVisualComp) {
        ItemVisualComp->OnOwnerDead();
    }

	// log server or client
	UE_LOG(LogTemp, Warning, TEXT("MulticastPlayerDeath called on %s"), HasAuthority() ? TEXT("Server") : TEXT("Client"));
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

    // if is hero, play sound hero dead
    if (GetCharacterRole() == ECharacterRole::Hero) {
        if (AudioComp) {
            AudioComp->PlayHeroDeath();
        }
    }

    if (IsLocallyControlled()) {
        AMyPlayerController* PC = Cast<AMyPlayerController>(GetController());
        if (!PC) {
            return;
        }
        PC->SetIgnoreLookInput(true);
        PC->SetIgnoreMoveInput(true);
        
        if (CameraComp->IsFPS()) {
            ChangeView(); // switch to tps view
        }
        if (!CachedCharacterAsset) {
            return;
        }
        if (!CachedCharacterAsset->DeathCameraProxyClass) return;

        // Use current camera position as start
        const FVector CamLoc = CameraTps->GetComponentLocation();
        const FRotator CamRot = CameraTps->GetComponentRotation();

        // Spawn proxy actor that has physics + camera
        DeathCameraProxy = GetWorld()->SpawnActor<AActor>(CachedCharacterAsset->DeathCameraProxyClass, CamLoc, CamRot);

        if (!DeathCameraProxy.IsValid()) {
            return;
		}
        // Add impulse so camera feels like it gets knocked down
        if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(DeathCameraProxy->GetRootComponent()))
        {
            Root->AddImpulse(FVector(
                FMath::RandRange(-80.f, 80.f),
                FMath::RandRange(-80.f, 80.f),
                -250.f
            ) * 20.f);
        }

        // Blend view to death camera
        PC->SetViewTargetWithBlend(DeathCameraProxy.Get(), 0.15f, VTBlend_EaseOut);
		UE_LOG(LogTemp, Warning, TEXT("Switched to death camera proxy."));

		// view spectator mode
        FTimerHandle TmpTimer;
        GetWorld()->GetTimerManager().SetTimer(
            TmpTimer,
            [PC]()
            {
               // PC->ServerSetSpectateTarget(false);
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
    if (IsFpsViewMode()) {
		return; // no blood fx in fps mode
    }
    if (!CachedCharacterAsset) {
        return;
    }
    const FRotator HitRotation = HitNormal.Rotation();
    if (CachedCharacterAsset->BloodFx) {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            CachedCharacterAsset->BloodFx,
            HitLocation
        );
    }
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
    if (AudioComp) {
        //   AudioComp->PlayBloodHit();
	}
}

void ABaseCharacter::PlayStunEffect(const float& Strength) 
{
    if (!CachedCharacterAsset) {
        return;
	}
    if (!FlashMID) {
        return;
    }
	
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("PlayStunEffectPlaying [STUN][%s] Character=%s Strength=%f"),
        HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
        *GetName(),
        Strength
    );

    if (CachedCharacterAsset->StunCurve)
    {
		UE_LOG(LogTemp, Warning, TEXT("PlayStunEffectPlaying stun effect with Strength: %f"), Strength);
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
        return;

    if (!bGrounded)
        return;

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

void ABaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    if (UGameManager* GM = UGameManager::Get(GetWorld()))
    {
        GM->UnregisterPlayer(this);
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

    UE_LOG(LogTemp, Warning, TEXT("DEUBGGGG: ABaseCharacter : BecomeViewTarget"));

    if (IsValid(CameraComp)) {
		UE_LOG(LogTemp, Warning, TEXT("DEUBGGGG: CameraComp is valid in BecomeViewTarget"));
        CameraComp->OnBecomeViewTarget(MyPC);
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("DEUBGGGG: CameraComp is null in BecomeViewTarget"));
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
		Move->bOrientRotationToMovement = true;
    }
}

void ABaseCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    //ApplyRotationMode(IsLocal, IsPlayer);
}

void ABaseCharacter::OnRep_Controller()
{
    Super::OnRep_Controller();

    /* bool IsLocal = Controller->IsLocalController();
    bool IsPlayer = Cast<APlayerController>(Controller) != nullptr;
    ApplyRotationMode(Cast<APlayerController>(Controller) != nullptr);*/
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
    // temp, refactor later
    if (NewRole == ECharacterRole::Hero) {
        if (InventoryComp) {
            InventoryComp->OnBecomeHero();
		}
        if (EquipComp) {
            EquipComp->SelectSlot(FGameConstants::SLOT_MELEE);
        }
        if (AudioComp) {
            AudioComp->PlayHeroSpawn();
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

	// role changed mean update walk speed
	UpdateMaxWalkSpeed();
}

void ABaseCharacter::ApplyVisualByRole(ECharacterRole NewRole)
{
	UE_LOG(LogTemp, Warning, TEXT("Applying visuals for role: %d"), (int32)NewRole);
    if (!CachedCharacterAsset)
        return;

    // Pick meshes/anims
    USkeletalMesh* NewTpsMesh = nullptr;
    USkeletalMesh* NewFpsMesh = nullptr;
    TSubclassOf<UAnimInstance> NewTpsAnim = nullptr;
    TSubclassOf<UAnimInstance> NewFpsAnim = nullptr;

    // hard code for testing
    //NewRole = ECharacterRole::Hero;
    if (NewRole == ECharacterRole::Zombie)
    {
        NewTpsMesh = CachedCharacterAsset->ZombieMeshTPS;
        NewFpsMesh = CachedCharacterAsset->ZombieMeshFPS;
        NewTpsAnim = CachedCharacterAsset->ZombieAnimTPS;
        NewFpsAnim = CachedCharacterAsset->ZombieAnimFPS;
    }
    else if (NewRole == ECharacterRole::Hero)
    {
        // Hero -> use team mesh
        NewTpsMesh = CachedCharacterAsset->HeroMeshTPS;
		NewFpsMesh = CachedCharacterAsset->HeroMeshFPS;

        NewTpsAnim = CachedCharacterAsset->HeroAnimTPS;
        NewFpsAnim = CachedCharacterAsset->HeroAnimFPS;

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
        NewTpsMesh = CachedCharacterAsset->CounterTerroristMesh;
		NewFpsMesh = CachedCharacterAsset->FpsMesh;
        NewTpsAnim = CachedCharacterAsset->HumanAnimTPS;    
        NewFpsAnim = CachedCharacterAsset->HumanAnimFPS;      
    }

    // Apply TPS
    if (USkeletalMeshComponent* TpsMesh = GetMesh())
    {
        if (NewTpsMesh && TpsMesh->GetSkeletalMeshAsset() != NewTpsMesh)
        {
            TpsMesh->SetSkeletalMesh(NewTpsMesh);
        }
        if (NewTpsAnim)
        {
            TpsMesh->SetAnimInstanceClass(NewTpsAnim);
        }
    }

    // Apply FPS
    if (MeshFps)
    {
        if (NewFpsMesh && MeshFps->GetSkeletalMeshAsset() != NewFpsMesh)
        {
            MeshFps->SetSkeletalMesh(NewFpsMesh);
        }
        if (NewFpsAnim)
        {
            MeshFps->SetAnimInstanceClass(NewFpsAnim);
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
    if (!CachedCharacterAsset || !CachedCharacterAsset->FlashPPMat || !FlashPP) return;

    FlashMID = UMaterialInstanceDynamic::Create(CachedCharacterAsset->FlashPPMat, this);
    FlashPP->Settings.AddBlendable(FlashMID, 1.0f);
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
        ZM->BecomeHero(this->Controller);
    }
}

void ABaseCharacter::SetupInitialInventory()
{
    if (!HasAuthority()) return;

    if (InventoryComp)
    {
        InventoryComp->Test();
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