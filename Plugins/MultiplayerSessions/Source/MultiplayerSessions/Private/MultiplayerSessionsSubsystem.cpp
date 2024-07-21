// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
  CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
  FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
  JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
  DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
  StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
  IOnlineSubsystem* subsystem = IOnlineSubsystem::Get();
  if (subsystem)
  {
    SessionInterface = subsystem->GetSessionInterface();
  }
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 numPublicConnections, FString matchType)
{
  if (!SessionInterface.IsValid()) return;

  // remove existing session if any
  auto existingSession = SessionInterface->GetNamedSession(NAME_GameSession);
  if (existingSession != nullptr)
  {
    SessionInterface->DestroySession(NAME_GameSession);
  }

  // session settings
  LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
  LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
  LastSessionSettings->NumPublicConnections = numPublicConnections;
  LastSessionSettings->bAllowJoinInProgress = true;
  LastSessionSettings->bAllowJoinViaPresence = true;
  LastSessionSettings->bUsesPresence = true;
  LastSessionSettings->bShouldAdvertise = true;
  LastSessionSettings->Set(FName("MatchType"), matchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

  // create session
  CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
  const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
  if (!SessionInterface->CreateSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
  {
    SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

    MultiplayerOnCreateSessionComplete.Broadcast(false);
  }
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 maxSearchResults)
{
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& sessionResult)
{
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName sessionName, bool bWasSuccessful)
{
  if (SessionInterface)
  {
    SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
  }
  MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result)
{
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName sessionName, bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName sessionName, bool bWasSuccessful)
{
}
