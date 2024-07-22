// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem() :
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
    bCreateSessionOnDestroy = true;
    LastNumPublicConnections = numPublicConnections;
    LastMatchType = matchType;

    DestroySession();
  }

  // session settings
  LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
  LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
  LastSessionSettings->NumPublicConnections = numPublicConnections;
  LastSessionSettings->bAllowJoinInProgress = true;
  LastSessionSettings->bAllowJoinViaPresence = true;
  LastSessionSettings->bUsesPresence = true;
  LastSessionSettings->bShouldAdvertise = true;
  LastSessionSettings->bUseLobbiesIfAvailable = true;
  LastSessionSettings->Set(FName("MatchType"), matchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
  LastSessionSettings->BuildUniqueId = 1;

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
  if (!SessionInterface.IsValid()) return;

  FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
  LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
  LastSessionSearch->MaxSearchResults = maxSearchResults;
  LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
  LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

  const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
  if (!SessionInterface->FindSessions(*localPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
  {
    SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

    MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
  }
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& sessionResult)
{
  if (!SessionInterface.IsValid())
  {
    MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
    return;
  }

  JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

  const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
  if (!SessionInterface->JoinSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, sessionResult))
  {
    SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

    MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
  }
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
  if (!SessionInterface.IsValid())
  {
    MultiplayerOnDestroySessionComplete.Broadcast(false);
    return;
  }

  DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

  if (!SessionInterface->DestroySession(NAME_GameSession))
  {
    SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
    MultiplayerOnDestroySessionComplete.Broadcast(false);
  }
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
  if (SessionInterface)
  {
    SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
  }

  if (LastSessionSearch->SearchResults.Num() <= 0)
  {
    MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
    return;
  }

  MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result)
{
  if (SessionInterface)
  {
    SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
  }

  MultiplayerOnJoinSessionComplete.Broadcast(result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName sessionName, bool bWasSuccessful)
{
  if (SessionInterface)
  {
    SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
  }
  if (bWasSuccessful && bCreateSessionOnDestroy)
  {
    bCreateSessionOnDestroy = false;
    CreateSession(LastNumPublicConnections, LastMatchType);
  }

  MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName sessionName, bool bWasSuccessful)
{
}
