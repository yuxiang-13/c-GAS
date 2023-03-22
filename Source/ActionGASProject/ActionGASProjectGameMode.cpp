// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionGASProjectGameMode.h"
#include "ActionGASProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

AActionGASProjectGameMode::AActionGASProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
