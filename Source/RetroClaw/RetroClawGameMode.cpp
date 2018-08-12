// Copyright Epic Games, Inc. All Rights Reserved.

#include "RetroClawGameMode.h"
#include "RetroClawCharacter.h"

ARetroClawGameMode::ARetroClawGameMode()
{
	// Set default pawn class to our character
	DefaultPawnClass = ARetroClawCharacter::StaticClass();	
}
