// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "../DebugHelper.h"

void ALobbyGameMode::PostLogin(APlayerController* newplayer)
{
  Super::PostLogin(newplayer);

  if (GameState)
  {
    int32  numOfPlayers = GameState.Get()->PlayerArray.Num();
    Debug::Print("Players in game: " + FString::FromInt(numOfPlayers), FColor::Yellow, 60.0f, 1);

    APlayerState* playerState = newplayer->GetPlayerState<APlayerState>();
    if (playerState)
    {
      FString playerName = playerState->GetPlayerName();
      Debug::Print(playerName + " has joined the game.", FColor::Cyan, 60.0f, -1);
    }
  }
}

void ALobbyGameMode::Logout(AController* exiting)
{
  Super::Logout(exiting);

  APlayerState* playerState = exiting->GetPlayerState<APlayerState>();
  if (playerState)
  {
    int32  numOfPlayers = GameState.Get()->PlayerArray.Num();
    Debug::Print("Players in game: " + FString::FromInt(numOfPlayers - 1), FColor::Yellow, 60.0f, 1);

    FString playerName = playerState->GetPlayerName();
    Debug::Print(playerName + " has quited exited the game.", FColor::Cyan, 60.0f, -1);
  }
}
