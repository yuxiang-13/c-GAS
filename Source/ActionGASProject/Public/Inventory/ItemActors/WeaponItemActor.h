// Fill out your copyright notice in the Description page of Project Settings.

#include "CoreMinimal.h"
#include "Actors/ItemActor.h"
#include "WeaponItemActor.generated.h"


/**
 * 
 */
UCLASS()
class ACTIONGASPROJECT_API AWeaponItemActor : public AItemActor
{
	GENERATED_BODY()
public:
	AWeaponItemActor();

	const UWeaponStaticData* GetWeaponStaticData() const;

	// 获取枪口位置
	UFUNCTION(BlueprintPure)
	FVector GetMuzzleLocation() const;

	// 一会这个函数用于 只在服务器执行
	UFUNCTION(BlueprintCallable)
	void PlayWeaponEffects(const FHitResult& InHitResult);
protected:
	// 一会这个函数用于 用于模拟玩家
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayWeaponEffects(const FHitResult& InHitResult);

	// 一会这个函数用于 在客户端执行
	void PlayWeaponEffectsIternal(const FHitResult& InHitResult);
	
	UPROPERTY()
	UMeshComponent* MeshComponent = nullptr;

	// 重写
	virtual void InitInternal() override;
};

#pragma once
