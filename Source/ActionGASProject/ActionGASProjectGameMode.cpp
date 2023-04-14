// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionGASProjectGameMode.h"
#include "ActionGASProjectCharacter.h"
#include "PlayerControllers/ActionGamePlayerController.h"
#include "UObject/ConstructorHelpers.h"

AActionGASProjectGameMode::AActionGASProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// 绑定PlayerControll
	PlayerControllerClass = AActionGamePlayerController::StaticClass();
}

void AActionGASProjectGameMode::NotifyPlayerDied(AActionGamePlayerController* PlayerController)
{
	if (PlayerController)
	{
		// 倒计时后进行复活
		PlayerController->RestartPlayerIn(2.f);
	}
}
