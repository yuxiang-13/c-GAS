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

protected:
	UPROPERTY()
	UMeshComponent* MeshComponent = nullptr;

	// 重写
	virtual void InitInternal() override;
};

#pragma once
