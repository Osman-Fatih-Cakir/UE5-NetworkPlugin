// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "MultiplayerSessions_HelperFunctions.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"

void UMenu::MenuSetup(int32 numOfPublicConnections, FString matchType)
{
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
      world->ServerTravel(FString("/Game/ThirdPerson/Maps/Lobby?listen"));
    }
  }
  else
  {
    MultiplayerSessionsDebug::Print("Failed to create session!", FColor::Red);
  }
}

void UMenu::HostButtonClicked()
{
  if (MultiplayerSessionsSubsystem)
  {
    MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
  }
}

void UMenu::JoinButtonClicked()
{
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
