// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AG_GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONGASPROJECT_API UAG_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
};
