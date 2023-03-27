// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/AG_GameInstance.h"
#include "GameplayAbilities/Public/AbilitySystemGlobals.h"

void UAG_GameInstance::Init()
{
	Super::Init();

	UAbilitySystemGlobals::Get().InitGlobalData();
}
