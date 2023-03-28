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
};
