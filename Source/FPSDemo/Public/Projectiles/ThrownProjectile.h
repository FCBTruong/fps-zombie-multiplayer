#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Weapons/WeaponData.h"
#include "ThrownProjectile.generated.h"

class UThrowableConfig;
UCLASS()
class FPSDEMO_API AThrownProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AThrownProjectile();

protected:
	bool bDidHit = false;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY() USphereComponent* Collision;
	UPROPERTY() UProjectileMovementComponent* Projectile;
	UPROPERTY() UStaticMeshComponent* WeaponMesh;
	const UThrowableConfig* Data;
	bool bIsExploded = false;

	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit);
	FTimerHandle TimerHandle_Explode;
	void ExplodeNow();
	virtual void OnExplode();
	
public:	
	void InitFromData(const UThrowableConfig* InData);
	void LaunchProjectile(FVector LaunchVelocity, AActor* InstigatorActor);
	UFUNCTION(NetMulticast, Unreliable) virtual void MulticastExplode(const FVector& Location);
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION(NetMulticast, Unreliable) void MulticastInitData(EItemId ItemId);
	const UThrowableConfig* GetWeaponData() { return Data; }
};