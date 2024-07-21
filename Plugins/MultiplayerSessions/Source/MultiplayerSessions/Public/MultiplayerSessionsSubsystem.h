// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);

/**
 *
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
  GENERATED_BODY()

public:
  UMultiplayerSessionsSubsystem();

  void CreateSession(int32 numPublicConnections, FString matchType);
  void FindSessions(int32 maxSearchResults);
  void JoinSession(const FOnlineSessionSearchResult& sessionResult);
  void DestroySession();
  void StartSession();

protected:
  void OnCreateSessionComplete(FName sessionName, bool bWasSuccessful);
  void OnFindSessionsComplete(bool bWasSuccessful);
  void OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result);
  void OnDestroySessionComplete(FName sessionName, bool bWasSuccessful);
  void OnStartSessionComplete(FName sessionName, bool bWasSuccessful);

public:
  // Delegates for callbacks for session creation info
  FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
  FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
  FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
  FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
  FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;


private:
  IOnlineSessionPtr SessionInterface = nullptr;
  TSharedPtr<FOnlineSessionSettings> LastSessionSettings = nullptr;
  TSharedPtr<FOnlineSessionSearch> LastSessionSearch = nullptr;

  FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
  FDelegateHandle CreateSessionCompleteDelegateHandle;

  FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
  FDelegateHandle FindSessionsCompleteDelegateHandle;

  FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
  FDelegateHandle JoinSessionCompleteDelegateHandle;

  FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
  FDelegateHandle DestroySessionCompleteDelegateHandle;

  FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
  FDelegateHandle StartSessionCompleteDelegateHandle;
};
