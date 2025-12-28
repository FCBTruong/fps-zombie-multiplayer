// WeaponFireComponent.cpp

#include "Components/WeaponFireComponent.h"
#include "Characters/BaseCharacter.h"
#include "Components/EquipComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/ItemVisualComponent.h"
#include "Components/ActionStateComponent.h" // adjust include to your path
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Damage/MyDamageType.h"
#include "Damage/MyPointDamageEvent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/FirearmConfig.h"
#include "Components/AnimationComponent.h"
#include "Components/CharAudioComponent.h"

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
		AudioComp = Character ? Character->GetAudioComponent() : nullptr;
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

	Super::EndPlay(EndPlayReason);
}

void UWeaponFireComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWeaponFireComponent, FireStartTimeServer);
}

void UWeaponFireComponent::Initialize(
	UEquipComponent* InEquip,
	UInventoryComponent* InInventory,
	UActionStateComponent* InAction,
	UItemVisualComponent* InVisual
)
{
	EquipComp = InEquip;
	InventoryComp = InInventory;
	ActionStateComp = InAction;
	VisualComp = InVisual;
	Character = Cast<ABaseCharacter>(GetOwner());

	if (EquipComp)
	{
		EquipComp->OnActiveItemChanged.AddUObject(this, &UWeaponFireComponent::OnActiveItemChanged);
	}
}

bool UWeaponFireComponent::IsOwningClient() const
{
	const ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	return OwnerChar && OwnerChar->IsLocallyControlled();
}

bool UWeaponFireComponent::CanFireNow() const
{
	if (!Character || !Character->IsAlive())
		return false;

	if (!EquipComp || !InventoryComp || !ActionStateComp)
		return false;

	if (!ActionStateComp->CanFireNow())
		return false;

	const EItemId WeaponId = EquipComp->GetActiveItemId();
	if (WeaponId == EItemId::NONE)
		return false;

	// Ammo check (only firearms should pass)
	const FWeaponState* State = InventoryComp->GetWeaponStateByItemId(WeaponId);
	if (!State)
		return false;

	return State->AmmoInClip > 0;
}

void UWeaponFireComponent::OnActiveItemChanged(EItemId /*NewId*/)
{
	// Stop any ongoing fire on weapon switch
	RequestStopFire();
	BurstAccDeg = 0.f;
	LastShotTime = 0.f;
}

void UWeaponFireComponent::RequestStartFire()
{
	UE_LOG(LogTemp, Log, TEXT("UWeaponFireComponent::RequestStartFire called"));
	if (!CanFireNow())
	{
		// if reason is bullet ammo empty, could trigger a "dry fire" sound/animation here
		if (InventoryComp && EquipComp) {
			const FWeaponState* State = InventoryComp->GetWeaponStateByItemId(EquipComp->GetActiveItemId());
			if (State) {
				if (State->AmmoInClip <= 0) {
					const UItemConfig* ItemConfig = EquipComp->GetActiveItemConfig();
					if (ItemConfig && ItemConfig->IsA(UFirearmConfig::StaticClass())) {
						// play sound
						const UFirearmConfig* FirearmConfig = Cast<UFirearmConfig>(ItemConfig);
						if (AudioComp) {
							AudioComp->PlaySound3D(FirearmConfig->DryFireSound);
						}
					}
					UE_LOG(LogTemp, Log, TEXT("Cannot fire: No ammo in clip"));
				}
			}
		}
		return;
	}

	BurstSeed = FMath::Rand();
#if !UE_SERVER
	// Client prediction (visual only)
	if (!GetOwner()->HasAuthority() && IsOwningClient())
	{
		ShotCount = 0;
		FireOnce_PredictedLocal();
		GetWorld()->GetTimerManager().SetTimer(
			FireTimer_Local,
			this,
			&UWeaponFireComponent::FireOnce_PredictedLocal,
			FireInterval,
			true
		);
	}
#endif

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
	BurstSeed = InBurstSeed;
	StartFire_ServerAuth();
}

void UWeaponFireComponent::ServerStopFire_Implementation()
{
	StopFire_ServerAuth();
}

void UWeaponFireComponent::StartFire_ServerAuth()
{
	if (!Character || !Character->HasAuthority())
		return;

	ShotCount = 0;

	// Seed determinism inputs for this firing sequence
	FireStartTimeServer = GetServerTimeSeconds();

	// First shot immediately, then loop
	FireOnce_ServerAuth();

	GetWorld()->GetTimerManager().SetTimer(
		FireTimer_Server,
		this,
		&UWeaponFireComponent::FireOnce_ServerAuth,
		FireInterval,
		true,
		FireInterval
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
}

#if !UE_SERVER
void UWeaponFireComponent::FireOnce_PredictedLocal()
{
	if (!CanFireNow())
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
	UpdateBurstSpreadOnShot(Now);

	FVector Start, AimDir;
	GetAim(Start, AimDir);

	const FVector ShotDir = ComputeShotDirDeterministic(AimDir, Now, BurstSeed);

	FHitResult Hit;
	FVector ShotEnd;
	TraceShot(Character, Start, ShotDir, Hit, ShotEnd);

	if (VisualComp)
	{
		VisualComp->PlayFireFX(ShotEnd);
	}

	LastShotTime = Now;
}
#endif

void UWeaponFireComponent::FireOnce_ServerAuth()
{
	if (!Character || !Character->HasAuthority())
		return;

	if (!CanFireNow())
	{
		StopFire_ServerAuth();
		return;
	}
	ShotCount++;

	const EItemId WeaponId = EquipComp->GetActiveItemId();

	// Consume ammo on server
	InventoryComp->ConsumeAmmo(WeaponId, 1);

	const float Now = GetServerTimeSeconds();
	if (LastShotTime > 0.f && (Now - LastShotTime) > BurstResetDelay)
	{
		BurstAccDeg = 0.f;
	}
	UpdateBurstSpreadOnShot(Now);

	FVector Start, AimDir;
	GetAim(Start, AimDir);

	const FVector ShotDir = ComputeShotDirDeterministic(AimDir, Now, BurstSeed);

	FHitResult Hit;
	FVector ShotEnd;
	const bool bHit = TraceShot(Character, Start, ShotDir, Hit, ShotEnd);

	if (bHit && Hit.GetActor())
	{
		UE_LOG(LogTemp, Log, TEXT("Hit actor: %s"), *Hit.GetActor()->GetName());
		FMyPointDamageEvent DamageEvent;
		DamageEvent.DamageTypeClass = UMyDamageType::StaticClass();
		DamageEvent.WeaponID = WeaponId;
		DamageEvent.bIsHeadshot = false;

		const float Damage = 25.f; // TODO: pull from weapon data by WeaponId
		Hit.GetActor()->TakeDamage(Damage, DamageEvent, Character->GetController(), nullptr);
	}

	MulticastFireFX(ShotEnd);

	LastShotTime = Now;
}

void UWeaponFireComponent::MulticastFireFX_Implementation(FVector_NetQuantize TargetPoint)
{
	if (IsOwningClient())
		return; // owning client already predicted

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
	const float SpreadRad = FMath::DegreesToRadians(SpreadDeg);

	return Stream.VRandCone(AimDir, SpreadRad, SpreadRad).GetSafeNormal();
}

float UWeaponFireComponent::GetTotalSpreadDeg(float NowServerTime) const
{
	// NOTE: BurstAccDeg should already be updated at the moment of firing on both sides.
	const float Total = Spread.BaseDeg + GetMovementSpreadDeg() + GetAirSpreadDeg() + BurstAccDeg;
	return FMath::Min(Total, Spread.MaxTotalDeg);
}

float UWeaponFireComponent::GetMovementSpreadDeg() const
{
	return Spread.MoveAddDeg * GetMoveAlphaForSpread();
}

float UWeaponFireComponent::GetAirSpreadDeg() const
{
	const ABaseCharacter* C = Cast<ABaseCharacter>(GetOwner());
	if (!C) return 0.0f;

	const UCharacterMovementComponent* Move = C->GetCharacterMovement();
	if (!Move) return 0.0f;

	return Move->IsFalling() ? Spread.AirAddDeg : 0.0f;
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
	const float Exp = FMath::Max(0.1f, Spread.MoveCurveExp);
	return FMath::Pow(Alpha, Exp);
}

void UWeaponFireComponent::UpdateBurstSpreadOnShot(float NowServerTime)
{
	// Deterministic recovery based on time gap since last shot
	if (LastShotTime > 0.0f)
	{
		const float Dt = FMath::Max(0.0f, NowServerTime - LastShotTime);
		BurstAccDeg = FMath::Max(0.0f, BurstAccDeg - Spread.BurstRecoverDegPerSec * Dt);
	}

	// Add per-shot burst spread
	BurstAccDeg = FMath::Min(BurstAccDeg + Spread.PerShotAddDeg, Spread.MaxBurstAddDeg);
	LastShotTime = NowServerTime;
}

void UWeaponFireComponent::RequestReload() {
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
	HandleReload();
}

bool UWeaponFireComponent::CanReload() const {
	if (!Character || !Character->IsAlive()) {
		return false;
	}
	if (!EquipComp || !InventoryComp) {
		return false;
	}
	if (!ActionStateComp) {
		return false;
	}
	if (EquipComp->GetActiveItemId() == EItemId::NONE) {
		return false;
	}

	const UItemConfig* ItemConf = EquipComp->GetActiveItemConfig();
	
	if (ItemConf->GetItemType() != EItemType::Firearm) {
		return false;
	}
	// check action state
	if (!ActionStateComp->CanReloadNow()) {
		return false;
	}

	// check ammo availability
	const FWeaponState* State = InventoryComp->GetWeaponStateByItemId(EquipComp->GetActiveItemId());
	if (!State) {
		return false;
	}
	if (State->AmmoInClip >= State->MaxAmmoInClip) {
		return false;
	}

	if (!State || State->AmmoReserve <= 0) {
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
	UE_LOG(LogTemp, Warning, TEXT("OnEquipWeaponFinished called"));
	ActionStateComp->TrySetState(EActionState::Reloading);

	MulticastReload();
	FTimerHandle TimerHandle_FinishReload;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_FinishReload,
		this,
		&UWeaponFireComponent::OnFinishedReload,
		2.0f,
		false
	);
}

void UWeaponFireComponent::MulticastReload_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("MulticastReload called"));
	if (!Character) {
		return;
	}
	if (!EquipComp) {
		return;
	}

	UAnimationComponent* AnimComp = Character->GetAnimationComponent();
	if (AnimComp) {
		// get item config to determine reload animation
		const UItemConfig* ItemConf = EquipComp->GetActiveItemConfig();
		if (!ItemConf) {
			return;
		}
		const UFirearmConfig* FirearmConf = Cast<UFirearmConfig>(ItemConf);
		if (FirearmConf)
		{
			if (FirearmConf->FirearmType == EFirearmType::Pistol)
			{
				AnimComp->PlayReloadPistolMontage();
			}
			else {
				AnimComp->PlayReloadRifleMontage();
			}
		}
	}

	/* if (AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(CurrentWeapon))
		{
			Firearm->PlayReloadSound();
		}*/
}

void UWeaponFireComponent::OnFinishedReload()
{
	if (!Character || !Character->IsAlive()) return;
	if (!GetOwner()->HasAuthority()) {
		return;
	}
	ActionStateComp->TrySetState(EActionState::Idle);

	UE_LOG(LogTemp, Warning, TEXT("OnFinishedReload called"));
	if (!EquipComp) {
		return;
	}
	InventoryComp->ReloadWeapon(EquipComp->GetActiveItemId());
}

bool UWeaponFireComponent::CanWeaponAim() const {
	if (!Character || !Character->IsAlive()) {
		return false;
	}
	if (!EquipComp || !InventoryComp) {
		return false;
	}
	if (!ActionStateComp) {
		return false;
	}
	
	const UItemConfig* ItemConf = EquipComp->GetActiveItemConfig();
	if (!ItemConf) {
		return false;
	}
	if (ItemConf->GetItemType() != EItemType::Firearm) {
		return false;
	}
	const UFirearmConfig* FirearmConf = Cast<UFirearmConfig>(ItemConf);
	if (!FirearmConf->bHasScopeEquipped) {
		return false;
	}
	return true;
}

// for server only
void UWeaponFireComponent::RequestFireOnce() {
	if (GetOwner()->HasAuthority())
	{
		if (CanFireNow())
		{
			FireOnce_ServerAuth();
		}
	}
}