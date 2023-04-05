// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionGameTypes.h"
#include "MotionWarpingComponent.h"
#include "AG_MotionWarpingComponent.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONGASPROJECT_API UAG_MotionWarpingComponent : public UMotionWarpingComponent
{
	GENERATED_BODY()
public:
	UAG_MotionWarpingComponent(const FObjectInitializer& ObjectInitializer);

	void SendWarpPointsToClients();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSyncWarpPoints(const TArray<FMotionWarpingTargetByLocationAndRotation>& Targets);
};
