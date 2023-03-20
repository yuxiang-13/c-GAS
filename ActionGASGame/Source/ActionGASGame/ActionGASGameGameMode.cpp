// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionGASGameGameMode.h"
#include "ActionGASGameCharacter.h"
#include "UObject/ConstructorHelpers.h"

AActionGASGameGameMode::AActionGASGameGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
