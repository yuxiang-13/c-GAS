// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ActionGASProjectGameMode.generated.h"

class AActionGamePlayerController;

UCLASS(minimalapi)
class AActionGASProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AActionGASProjectGameMode();

	// 倒计时后进行复活
	void NotifyPlayerDied(AActionGamePlayerController* PlayerController);
};



