// WeaponFireComponent.cpp

#include "Game/Characters/Components/WeaponFireComponent.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/Characters/Components/EquipComponent.h"
#include "Game/Characters/Components/InventoryComponent.h"
#include "Game/Characters/Components/ItemVisualComponent.h"
#include "Game/Characters/Components/ActionStateComponent.h" // adjust include to your path
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Game/Utils/Damage/MyDamageType.h"
#include "Game/Utils/Damage/MyPointDamageEvent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Shared/Data/Items/FirearmConfig.h"
#include "Game/Characters/Components/AnimationComponent.h"
#include "Game/Characters/Components/CharAudioComponent.h"
#include "Shared/System/ItemsManager.h"
#include "Game/Utils/Damage/DamageHelpers.h"

UWeaponFireComponent::UWeaponFireComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UWeaponFireComponent::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("UWeaponFireComponent::BeginPlay called"));
	Character = Cast<ABaseCharacter>(GetOwner());

	if (Character) {
		InventoryComp = Character->GetInventoryComponent();
		ActionStateComp = Character->GetActionStateComponent();
		VisualComp = Character->GetItemVisualComponent();

		AudioComp = Character->GetAudioComponent();
	}
}

void UWeaponFireComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if !UE_SERVER
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimer_Local);
	}
#endif
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimer_Server);
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
	}

	Super::EndPlay(EndPlayReason);
}

void UWeaponFireComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWeaponFireComponent, FireStartTimeServer);
}

bool UWeaponFireComponent::IsOwningClient() const
{
	const ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	return OwnerChar && OwnerChar->IsLocallyControlled();
}

EFireEnableReason UWeaponFireComponent::CanFireNow() const
{
	if (!IsEnabled())
		return EFireEnableReason::Undefined;
	if (!CurrentFirearmConfig) {
		return EFireEnableReason::Undefined;
	}
	if (!Character || !Character->IsAlive())
		return EFireEnableReason::Undefined;
	if (!InventoryComp || !ActionStateComp)
		return EFireEnableReason::Undefined;
	if (!ActionStateComp->CanFireNow())
	{
		return EFireEnableReason::Undefined;
	}

	float TimeSinceLastShot = GetServerTimeSeconds() - LastShotTime;
	if (TimeSinceLastShot < CurrentFirearmConfig->FireInterval * 0.8) // allow some leeway for timer inaccuracies
		return EFireEnableReason::Undefined;

	// Ammo check (only firearms should pass)
	const FWeaponState* State = InventoryComp->GetWeaponStateByItemId(CurrentFirearmConfig->Id);
	if (!State)
		return EFireEnableReason::Undefined;
	if (State->AmmoInClip <= 0)
		return EFireEnableReason::NoAmmo;

	return EFireEnableReason::OK;
}

void UWeaponFireComponent::OnActiveItemChanged(EItemId NewId)
{
#if !UE_SERVER
	if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(FireTimer_Local);
#endif
	if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(FireTimer_Server);

	if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);

	if (NewId == EItemId::NONE)
	{
		CurrentFirearmConfig = nullptr;
		return;
	}
	CurrentFirearmConfig = Cast<UFirearmConfig>(UItemsManager::Get(GetWorld())->GetItemById(NewId));
	BurstAccDeg = 0.f;
	LastShotTime = 0.f;
}

void UWeaponFireComponent::RequestStartFire()
{
	if (!IsEnabled())
		return;

	UE_LOG(LogTemp, Log, TEXT("UWeaponFireComponent::RequestStartFire called"));
	EFireEnableReason Reason = CanFireNow();
	if (Reason != EFireEnableReason::OK)
	{
		// if reason is bullet ammo empty, could trigger a "dry fire" sound/animation here

		if (Reason == EFireEnableReason::NoAmmo) {
			if (AudioComp) {
				AudioComp->PlaySound3D(CurrentFirearmConfig->DryFireSound);
			}
		}
		UE_LOG(LogTemp, Log, TEXT("Cannot start fire, reason: %d"), static_cast<int>(Reason));
		return;
	}

	BurstSeed = FMath::Rand();
#if !UE_SERVER
	// Client prediction (visual only)
	if (!GetOwner()->HasAuthority() && IsOwningClient())
	{
		// should reset spread state on start
		ShotCount = 0;
		BurstAccDeg = 0;
		FireOnce_PredictedLocal();

		GetWorld()->GetTimerManager().SetTimer(
			FireTimer_Local,
			this,
			&UWeaponFireComponent::FireOnce_PredictedLocal,
			CurrentFirearmConfig->FireInterval,
			true
		);
	}
#endif
	// if is aiming, stop aim
	ABaseCharacter* Char = Cast<ABaseCharacter>(GetOwner());
	if (Char && Char->IsAiming()) {
		Char->RequestStopAiming();
	}

	if (GetOwner()->HasAuthority())
	{
		StartFire_ServerAuth();
	}
	else
	{
		ServerStartFire(BurstSeed);
	}
}

void UWeaponFireComponent::RequestStopFire()
{
#if !UE_SERVER
	if (!GetOwner()->HasAuthority() && IsOwningClient() && GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimer_Local);
	}
#endif

	if (GetOwner()->HasAuthority())
	{
		StopFire_ServerAuth();
	}
	else
	{
		ServerStopFire();
	}
}

void UWeaponFireComponent::ServerStartFire_Implementation(int InBurstSeed)
{
	if (!IsEnabled())
		return;
	BurstSeed = InBurstSeed;
	StartFire_ServerAuth();
}

void UWeaponFireComponent::ServerStopFire_Implementation()
{
	StopFire_ServerAuth();
}

void UWeaponFireComponent::StartFire_ServerAuth()
{
	if (!IsEnabled())
		return;
	if (!Character || !Character->HasAuthority())
		return;

	if (!CurrentFirearmConfig) return;
	if (ActionStateComp->IsInState(EActionState::Firing)) // already firing
		return;
	if (CanFireNow() != EFireEnableReason::OK)
		return;

	ActionStateComp->TrySetState(EActionState::Firing);

	ShotCount = 0;
	BurstAccDeg = 0;

	// Seed determinism inputs for this firing sequence
	FireStartTimeServer = GetServerTimeSeconds();

	// First shot immediately, then loop
	FireOnce_ServerAuth();

	GetWorld()->GetTimerManager().SetTimer(
		FireTimer_Server,
		this,
		&UWeaponFireComponent::FireOnce_ServerAuth,
		CurrentFirearmConfig->FireInterval,
		true,
		CurrentFirearmConfig->FireInterval
	);
}

void UWeaponFireComponent::StopFire_ServerAuth()
{
	if (!Character || !Character->HasAuthority())
		return;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimer_Server);
	}
	if (ActionStateComp->IsInState(EActionState::Firing)) {
		ActionStateComp->TrySetState(EActionState::Idle);
	}
}

#if !UE_SERVER
void UWeaponFireComponent::FireOnce_PredictedLocal()
{
	if (CanFireNow() != EFireEnableReason::OK)
	{
		if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(FireTimer_Local);
		return;
	}
	ShotCount++;

	const float Now = GetServerTimeSeconds();
	if (LastShotTime > 0.f && (Now - LastShotTime) > BurstResetDelay)
	{
		BurstAccDeg = 0.f;
	}

	FVector Start, AimDir;
	GetAim(Start, AimDir);

	const FVector ShotDir = ComputeShotDirDeterministic(AimDir, Now, BurstSeed);
	UpdateBurstSpreadOnShot(Now);

	FHitResult Hit;
	FVector ShotEnd;
	TraceShot(Character, Start, ShotDir, Hit, ShotEnd);

	if (VisualComp)
	{
		VisualComp->PlayFireFX(ShotEnd);
	}

	LastShotTime = Now;

	ApplyRecoilLocal(); // camera jitter
}
#endif

void UWeaponFireComponent::FireOnce_ServerAuth()
{
	UE_LOG(LogTemp, Log, TEXT("UWeaponFireComponent::FireOnce_ServerAuth called"));
	if (!Character || !Character->HasAuthority()) {
		UE_LOG(LogTemp, Log, TEXT("UWeaponFireComponent::FireOnce_ServerAuth called012"));
		return;
	}

	if (CanFireNow() != EFireEnableReason::OK)
	{
		UE_LOG(LogTemp, Log, TEXT("UWeaponFireComponent::FireOnce_ServerAuth called01"));
		StopFire_ServerAuth();
		return;
	}
	ShotCount++;

	// Consume ammo on server
	InventoryComp->ConsumeAmmo(CurrentFirearmConfig->Id, 1);

	const float Now = GetServerTimeSeconds();
	if (LastShotTime > 0.f && (Now - LastShotTime) > BurstResetDelay)
	{
		BurstAccDeg = 0.f;
	}

	FVector Start, AimDir;
	GetAim(Start, AimDir);

	const FVector ShotDir = ComputeShotDirDeterministic(AimDir, Now, BurstSeed);
	UpdateBurstSpreadOnShot(Now);

	FHitResult Hit;
	FVector ShotEnd;
	const bool bHit = TraceShot(Character, Start, ShotDir, Hit, ShotEnd);

	if (bHit && Hit.GetActor())
	{
		FDamageApplyParams Params;
		Params.BaseDamage = CurrentFirearmConfig->Damage;
		Params.WeaponId = CurrentFirearmConfig->Id;
		Params.DamageTypeClass = UMyDamageType::StaticClass();
		Params.bEnableHeadshot = true;
		Params.Hit = Hit;

		DamageHelpers::ApplyMyPointDamage(
			Hit.GetActor(),
			Params,
			Character->GetController(),
			nullptr
		);
	}
	UE_LOG(LogTemp, Log, TEXT("UWeaponFireComponent::FireOnce_ServerAuth called0001"));
	MulticastFireFX(ShotEnd);
	LastShotTime = Now;

#if !UE_SERVER
	if (IsOwningClient()) // listen server host
	{
		if (VisualComp) {
			VisualComp->PlayFireFX(ShotEnd);
		}
		ApplyRecoilLocal();
	}
#endif
}

void UWeaponFireComponent::MulticastFireFX_Implementation(FVector_NetQuantize TargetPoint)
{
	if (IsOwningClient())
		return;

	if (VisualComp)
	{
		VisualComp->PlayFireFX(TargetPoint);
	}
}

void UWeaponFireComponent::GetAim(FVector& OutStart, FVector& OutDir) const
{
	OutStart = Character ? Character->GetActorLocation() : FVector::ZeroVector;
	OutDir = Character ? Character->GetActorForwardVector() : FVector::ForwardVector;

	if (Character)
	{
		FRotator ViewRot;
		Character->GetActorEyesViewPoint(OutStart, ViewRot);
		OutDir = ViewRot.Vector();
	}
}

bool UWeaponFireComponent::TraceShot(
	const AActor* IgnoredActor,
	const FVector& Start,
	const FVector& Dir,
	FHitResult& OutHit,
	FVector& OutEnd
) const
{
	UWorld* World = GetWorld();
	if (!World) return false;

	OutEnd = Start + Dir * 10000.f;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponTrace), false);
	if (IgnoredActor) Params.AddIgnoredActor(IgnoredActor);

	const bool bHit = World->LineTraceSingleByChannel(OutHit, Start, OutEnd, ECC_Visibility, Params);

	UE_LOG(LogTemp, Log, TEXT("TraceShot: Start=%s, Dir=%s, End=%s, Hit=%s"),
		*Start.ToString(), *Dir.ToString(), *OutEnd.ToString(),
		bHit ? *OutHit.ImpactPoint.ToString() : TEXT("None"));
	if (bHit) OutEnd = OutHit.ImpactPoint;

	return bHit;
}

float UWeaponFireComponent::GetServerTimeSeconds() const
{
	const UWorld* World = GetWorld();
	if (!World) return 0.f;

	const AGameStateBase* GS = World->GetGameState();
	return GS ? GS->GetServerWorldTimeSeconds() : World->GetTimeSeconds();
}

int32 UWeaponFireComponent::ComputeShotIndex(float NowServerTime) const
{
	return ShotCount;
	/*if (FireInterval <= 0.f) return 0;

	const float Elapsed = FMath::Max(0.f, NowServerTime - FireStartTimeServer);
	const float Eps = 0.0001f;

	return FMath::Max(0, FMath::FloorToInt((Elapsed + Eps) / FireInterval));*/
}

FVector UWeaponFireComponent::ComputeShotDirDeterministic(
	const FVector& AimDir,
	float NowServerTime,
	int32 Seed
) const
{
	const int32 ShotIndex = ComputeShotIndex(NowServerTime);
	const int32 PerShotSeed = Seed ^ (ShotIndex * 196613);

	FRandomStream Stream(PerShotSeed);

	const float SpreadDeg = GetTotalSpreadDeg(NowServerTime);
	float SpreadRad = FMath::DegreesToRadians(SpreadDeg);

	// if crouching, reduce spread by half
	if (Character && Character->bIsCrouched) {
		SpreadRad *= 0.5f;
	}

	return Stream.VRandCone(AimDir, SpreadRad, SpreadRad).GetSafeNormal();
}

float UWeaponFireComponent::GetTotalSpreadDeg(float NowServerTime) const
{
	// NOTE: BurstAccDeg should already be updated at the moment of firing on both sides.
	float MoveSpreadDeg = GetMovementSpreadDeg();
	const float Total = CurrentFirearmConfig->BaseDeg + MoveSpreadDeg + GetAirSpreadDeg() + BurstAccDeg;
	return FMath::Min(Total, CurrentFirearmConfig->MaxTotalDeg);
}

float UWeaponFireComponent::GetMovementSpreadDeg() const
{
	return CurrentFirearmConfig->MoveAddDeg * GetMoveAlphaForSpread();
}

float UWeaponFireComponent::GetAirSpreadDeg() const
{
	const ABaseCharacter* C = Cast<ABaseCharacter>(GetOwner());
	if (!C) return 0.0f;

	const UCharacterMovementComponent* Move = C->GetCharacterMovement();
	if (!Move) return 0.0f;

	return Move->IsFalling() ? CurrentFirearmConfig->AirAddDeg : 0.0f;
}

float UWeaponFireComponent::GetMoveAlphaForSpread() const
{
	// Use movement input magnitude for better client/server match.
	// This exists on owning client and on server (server receives movement input).
	const AActor* C = GetOwner();
	if (!C) return 0.0f;

	const float Speed2D = C->GetVelocity().Size2D();
	const float Alpha = FMath::Clamp(Speed2D / 600, 0.0f, 1.0f); // 600 is max walk speed

	// Apply curve
	const float Exp = FMath::Max(0.1f, CurrentFirearmConfig->MoveCurveExp);
	return FMath::Pow(Alpha, Exp);
}

void UWeaponFireComponent::UpdateBurstSpreadOnShot(float NowServerTime)
{
	// Add per-shot burst spread
	BurstAccDeg = FMath::Min(BurstAccDeg + CurrentFirearmConfig->PerShotAddDeg, CurrentFirearmConfig->MaxBurstAddDeg);
	LastShotTime = NowServerTime;
}

void UWeaponFireComponent::RequestReload() {
	if (!IsEnabled()) return;

	UE_LOG(LogTemp, Log, TEXT("UWeaponFireComponent: Reload called"));
	if (!Character || !Character->IsAlive()) return;
	if (!CanReload()) {
		UE_LOG(LogTemp, Log, TEXT("UWeaponFireComponent: Cannot Reload"));
		return;
	}

	if (GetOwner()->HasAuthority()) {
		HandleReload();
	}
	else
	{
		ServerReload();
	}
}

void UWeaponFireComponent::ServerReload_Implementation()
{
	if (!IsEnabled()) return;
	HandleReload();
}

bool UWeaponFireComponent::CanReload() const {
	if (!IsEnabled()) {
		return false;
	}

	if (!Character || !Character->IsAlive()) {
		return false;
	}
	if (!InventoryComp) {
		return false;
	}
	if (!ActionStateComp) {
		return false;
	}
	if (!CurrentFirearmConfig) {
		return false;
	}

	// check action state
	if (!ActionStateComp->CanReloadNow()) {
		UE_LOG(LogTemp, Log, TEXT("UWeaponFireComponent: Cannot Reload - now"));
		return false;
	}

	// check ammo availability
	const FWeaponState* State = InventoryComp->GetWeaponStateByItemId(CurrentFirearmConfig->Id);
	if (!State) {
		return false;
	}
	if (State->AmmoInClip >= State->MaxAmmoInClip) {
		UE_LOG(LogTemp, Log, TEXT("UWeaponFireComponent: Cannot Reload - max"));
		return false;
	}

	if (!State || State->AmmoReserve <= 0) {
		UE_LOG(LogTemp, Log, TEXT("UWeaponFireComponent: Cannot Reload - ammo reserver"));
		return false;
	}

	return true;
}

// Server function
void UWeaponFireComponent::HandleReload()
{
	if (!CanReload()) {
		return;
	}
	StopFire_ServerAuth();

	UE_LOG(LogTemp, Warning, TEXT("OnEquipWeaponFinished called"));
	ActionStateComp->TrySetState(EActionState::Reloading);

	MulticastReload();

	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimer,
		this,
		&UWeaponFireComponent::HandleFinishedReload,
		2.5f,
		false
	);
}

void UWeaponFireComponent::MulticastReload_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("MulticastReload called"));
	if (!Character) {
		return;
	}

	if (!CurrentFirearmConfig) {
		return;
	}

	UAnimationComponent* AnimComp = Character->GetAnimationComponent();
	if (AnimComp) {
		// get item config to determine reload animation

		if (CurrentFirearmConfig->FirearmType == EFirearmType::Pistol)
		{
			AnimComp->PlayReloadPistolMontage();
		}
		else {
			AnimComp->PlayReloadRifleMontage();
		}
	}

	if (AudioComp) {
		AudioComp->PlaySound3D(CurrentFirearmConfig->ReloadMagSound);
	}
}

void UWeaponFireComponent::HandleFinishedReload()
{
	if (!Character || !Character->IsAlive()) return;
	if (!GetOwner()->HasAuthority()) {
		return;
	}
	if (!CurrentFirearmConfig) {
		return;
	}
	if (ActionStateComp->GetState() != EActionState::Reloading) {
		return;
	}
	ActionStateComp->TrySetState(EActionState::Idle);

	UE_LOG(LogTemp, Warning, TEXT("OnFinishedReload called"));
	InventoryComp->ReloadWeapon(CurrentFirearmConfig->Id);

	OnFinishedReload.Broadcast();
}

bool UWeaponFireComponent::CanWeaponAim() const {
	if (!IsEnabled()) {
		return false;
	}

	if (!Character || !Character->IsAlive()) {
		return false;
	}
	if (!CurrentFirearmConfig) {
		return false;
	}
	if (!InventoryComp) {
		return false;
	}
	if (!ActionStateComp) {
		return false;
	}

	if (!CurrentFirearmConfig->bHasScopeEquipped) {
		return false;
	}
	return true;
}

void UWeaponFireComponent::ApplyRecoilLocal()
{
#if !UE_SERVER
	if (!Character) return;
	if (!IsOwningClient()) return;

	if (ShotCount == 1) {
		// no recoil on first shot
		return;
	}

	const float PitchKick = CurrentFirearmConfig->RecoilPitchPerShot + FMath::FRandRange(-CurrentFirearmConfig->RecoilPitchJitter, CurrentFirearmConfig->RecoilPitchJitter);
	const float YawKick = FMath::FRandRange(-CurrentFirearmConfig->RecoilYawPerShot, CurrentFirearmConfig->RecoilYawPerShot);

	Character->AddControllerPitchInput(-PitchKick); // look up a bit
	Character->AddControllerYawInput(YawKick);     // slight horizontal recoil
#endif
}


void UWeaponFireComponent::OnEnabledChanged(bool bNowEnabled)
{
	if (!GetWorld()) return;

#if !UE_SERVER
	GetWorld()->GetTimerManager().ClearTimer(FireTimer_Local);
#endif
	GetWorld()->GetTimerManager().ClearTimer(FireTimer_Server);
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);

	if (!bNowEnabled)
	{
		ShotCount = 0;
		BurstAccDeg = 0.f;
		LastShotTime = 0.f;
	}
}