// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "AG_PhysicalMaterial.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONGASPROJECT_API UAG_PhysicalMaterial : public UPhysicalMaterial
{
	GENERATED_BODY()
public:
	// 脚步声
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PhysicalMaterial")
	class USoundBase* FootstepSound = nullptr;
	// 子弹撞击声
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PhysicalMaterial")
	class USoundBase* PointImpactSound = nullptr;
	// 子弹撞击特效
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PhysicalMaterial")
	class UNiagaraSystem* PointImpactVFX = nullptr;
};
