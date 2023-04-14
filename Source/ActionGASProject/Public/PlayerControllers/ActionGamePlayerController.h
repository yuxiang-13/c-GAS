// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "ActionGamePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONGASPROJECT_API AActionGamePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	// 倒计时后复活角色
	void RestartPlayerIn(float InTime);
	// 复活角色
	void RestartPlayer();
	

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// 死亡 EventTag 触发
	UFUNCTION()
	void OnPawnDeathStateChanged(const FGameplayTag CallbackTag, int32 NewCount);
	
protected:
	FTimerHandle RestartPlayerTimerHandle;
	FDelegateHandle DeathStateTagDelegate;
};
