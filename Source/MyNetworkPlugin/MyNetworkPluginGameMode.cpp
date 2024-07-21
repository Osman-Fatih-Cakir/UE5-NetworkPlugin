// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyNetworkPluginGameMode.h"
#include "MyNetworkPluginCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMyNetworkPluginGameMode::AMyNetworkPluginGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
