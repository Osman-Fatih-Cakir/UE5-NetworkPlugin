// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "MultiplayerSessions_HelperFunctions.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

void UMenu::MenuSetup(int32 numOfPublicConnections, FString matchType, FString lobbyPath)
{
  PathToLobby = FString::Printf(TEXT("%s?listen"), *lobbyPath);
  NumPublicConnections = numOfPublicConnections;
  MatchType = matchType;

  AddToViewport();
  SetVisibility(ESlateVisibility::Visible);
  bIsFocusable = true;

  UWorld* world = GetWorld();
  if (world)
  {
    APlayerController* playerController = world->GetFirstPlayerController();
    if (playerController)
    {
      FInputModeUIOnly InputModeData;
      InputModeData.SetWidgetToFocus(TakeWidget());
      InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
      playerController->SetInputMode(InputModeData);
      playerController->SetShowMouseCursor(true);
    }
  }

  UGameInstance* gameInstance = GetGameInstance();
  if (gameInstance)
  {
    MultiplayerSessionsSubsystem = gameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
  }

  if (MultiplayerSessionsSubsystem)
  {
    MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
    MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
    MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
    MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
    MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
  }
}

bool UMenu::Initialize()
{
  if (!Super::Initialize()) return false;

  if (HostButton)
  {
    HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
  }

  if (JoinButton)
  {
    JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
  }

  return true;
}

void UMenu::NativeDestruct()
{
  MenuTeardown();

  Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
  if (bWasSuccessful)
  {
    MultiplayerSessionsDebug::Print("Session created successfully", FColor::Green);

    UWorld* world = GetWorld();
    if (world)
    {
      world->ServerTravel(PathToLobby);
    }
  }
  else
  {
    MultiplayerSessionsDebug::Print("Failed to create session!", FColor::Red);
    HostButton->SetIsEnabled(true);
  }
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
  if (!MultiplayerSessionsSubsystem) return;

  for (const FOnlineSessionSearchResult& sessionResult : SessionResults)
  {
    FString settingsValue;
    sessionResult.Session.SessionSettings.Get(FName("MatchType"), settingsValue);
    if (settingsValue == MatchType)
    {
      MultiplayerSessionsSubsystem->JoinSession(sessionResult);
      return;
    }
  }
  if (!bWasSuccessful || SessionResults.Num() <= 0)
  {
    JoinButton->SetIsEnabled(true);
  }
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
  IOnlineSubsystem* subsystem = IOnlineSubsystem::Get();
  if (subsystem)
  {
    IOnlineSessionPtr sessionInterface = subsystem->GetSessionInterface();
    if (sessionInterface.IsValid())
    {
      FString address;
      sessionInterface->GetResolvedConnectString(NAME_GameSession, address);

      APlayerController* playerController = GetGameInstance()->GetFirstLocalPlayerController();
      if (playerController)
      {
        playerController->ClientTravel(address, ETravelType::TRAVEL_Absolute);
      }
    }
  }
  if (Result != EOnJoinSessionCompleteResult::Success)
  {
    JoinButton->SetIsEnabled(true);
  }
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::HostButtonClicked()
{
  HostButton->SetIsEnabled(false);
  if (MultiplayerSessionsSubsystem)
  {
    MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
  }
}

void UMenu::JoinButtonClicked()
{
  JoinButton->SetIsEnabled(false);
  if (MultiplayerSessionsSubsystem)
  {
    MultiplayerSessionsSubsystem->FindSessions(10000);
  }
}

void UMenu::MenuTeardown()
{
  RemoveFromParent();

  UWorld* world = GetWorld();
  if (world)
  {
    APlayerController* playerController = world->GetFirstPlayerController();
    if (playerController)
    {
      FInputModeGameOnly InputModeData;
      playerController->SetInputMode(InputModeData);
      playerController->SetShowMouseCursor(false);
    }
  }
}
