// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

class UButton;
class UMultiplayerSessionsSubsystem;

/**
 *
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintCallable)
  void MenuSetup(int32 numOfPublicConnections = 4, FString matchType = FString(TEXT("FreeForAll")), FString lobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")));

protected:
  virtual bool Initialize() override;
  virtual void NativeDestruct() override;

  // Callbacks for the delegetes on MultiplayerSessionsSubsystem
  UFUNCTION()
  void OnCreateSession(bool bWasSuccessful);
  void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
  void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
  UFUNCTION()
  void OnDestroySession(bool bWasSuccessful);
  UFUNCTION()
  void OnStartSession(bool bWasSuccessful);

private:
  UFUNCTION()
  void HostButtonClicked();
  UFUNCTION()
  void JoinButtonClicked();

  void MenuTeardown();

private:
  UPROPERTY(meta = (BindWidget))
  UButton* HostButton = nullptr;
  UPROPERTY(meta = (BindWidget))
  UButton* JoinButton = nullptr;

  UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem = nullptr;

  int32 NumPublicConnections{ 4 };
  FString MatchType{ TEXT("FreeForAll") };
  FString PathToLobby{ TEXT("") };
};
