// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionGameTypes.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class ACTIONGASPROJECT_API AProjectile : public AActor
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Replicated)
	TSubclassOf<UProjectilesStaticData> ProjectileDataClass;
public:	
	AProjectile();
	
	// 返回静态数据
	const UProjectilesStaticData* GetProjectilesStaticData() const;
protected:
	// 抛体组件
	UPROPERTY()
	class UProjectileMovementComponent* ProjectileMovementComponent = nullptr;
	
	// 静态网格
	UPROPERTY()
	class UStaticMeshComponent* StaticMeshComponent = nullptr;
protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 绑定 抛体组件 停止监听
	UFUNCTION()
	void OnProjectileStop(const FHitResult& ImpactResult);
	
	// Debug画出抛物线  方便看落点哪里
	void DebugDrawPath() const;

};
