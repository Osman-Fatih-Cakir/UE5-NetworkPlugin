// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyNetworkPluginCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "DebugHelper.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AMyNetworkPluginCharacter

AMyNetworkPluginCharacter::AMyNetworkPluginCharacter() :
  CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
  FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
  JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete))
{
  // Set size for collision capsule
  GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

  // Don't rotate when the controller rotates. Let that just affect the camera.
  bUseControllerRotationPitch = false;
  bUseControllerRotationYaw = false;
  bUseControllerRotationRoll = false;

  // Configure character movement
  GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
  GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

  // Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
  // instead of recompiling to adjust them
  GetCharacterMovement()->JumpZVelocity = 700.f;
  GetCharacterMovement()->AirControl = 0.35f;
  GetCharacterMovement()->MaxWalkSpeed = 500.f;
  GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
  GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
  GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

  // Create a camera boom (pulls in towards the player if there is a collision)
  CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
  CameraBoom->SetupAttachment(RootComponent);
  CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
  CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

  // Create a follow camera
  FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
  FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
  FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

  // Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
  // are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

  IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
  if (onlineSubsystem)
  {
    OnlineSessionInterface = onlineSubsystem->GetSessionInterface();

    Debug::Print("Found subsytem " + onlineSubsystem->GetSubsystemName().ToString(), FColor::Green, 15.0f);
  }
}

void AMyNetworkPluginCharacter::BeginPlay()
{
  // Call the base class  
  Super::BeginPlay();
}

#pragma region CharacterThings
//////////////////////////////////////////////////////////////////////////
// Input

void AMyNetworkPluginCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
  // Add Input Mapping Context
  if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
  {
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
    {
      Subsystem->AddMappingContext(DefaultMappingContext, 0);
    }
  }

  // Set up action bindings
  if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

    // Jumping
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

    // Moving
    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyNetworkPluginCharacter::Move);

    // Looking
    EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyNetworkPluginCharacter::Look);
  }
  else
  {
    UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
  }
}

void AMyNetworkPluginCharacter::Move(const FInputActionValue& Value)
{
  // input is a Vector2D
  FVector2D MovementVector = Value.Get<FVector2D>();

  if (Controller != nullptr)
  {
    // find out which way is forward
    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);

    // get forward vector
    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

    // get right vector 
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    // add movement 
    AddMovementInput(ForwardDirection, MovementVector.Y);
    AddMovementInput(RightDirection, MovementVector.X);
  }
}

void AMyNetworkPluginCharacter::Look(const FInputActionValue& Value)
{
  // input is a Vector2D
  FVector2D LookAxisVector = Value.Get<FVector2D>();

  if (Controller != nullptr)
  {
    // add yaw and pitch input to controller
    AddControllerYawInput(LookAxisVector.X);
    AddControllerPitchInput(LookAxisVector.Y);
  }
}
#pragma endregion

void AMyNetworkPluginCharacter::CreateGameSession()
{
  if (!OnlineSessionInterface.IsValid()) return;

  // Delete the session if exists already
  auto existingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
  if (existingSession != nullptr)
  {
    OnlineSessionInterface->DestroySession(NAME_GameSession);

    Debug::Print("Session named: NAME_GameSession is destroyed.", FColor::Blue, 15.0f);
  }

  OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

  // Create a new session
  TSharedPtr<FOnlineSessionSettings> sessionSettings = MakeShareable(new FOnlineSessionSettings());
  sessionSettings->bIsLANMatch = false;
  sessionSettings->NumPublicConnections = 4;
  sessionSettings->bAllowJoinInProgress = true;
  sessionSettings->bAllowJoinViaPresence = true;
  sessionSettings->bUsesPresence = true;
  sessionSettings->bShouldAdvertise = true;
  sessionSettings->bUseLobbiesIfAvailable = true;
  sessionSettings->Set(FName("MatchType"), FString("FreeForAll"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

  const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
  OnlineSessionInterface->CreateSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *sessionSettings);
}

void AMyNetworkPluginCharacter::JoinGameSession()
{
  // Find game sessions
  if (!OnlineSessionInterface.IsValid()) return;

  OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

  SessionSearch = MakeShareable(new FOnlineSessionSearch());
  SessionSearch->MaxSearchResults = 10000;
  SessionSearch->bIsLanQuery = false;
  SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

  const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
  OnlineSessionInterface->FindSessions(*localPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
}

void AMyNetworkPluginCharacter::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
  if (bWasSuccessful)
  {
    Debug::Print("Created session: " + SessionName.ToString(), FColor::Green, 15.0f);

    UWorld* world = GetWorld();
    if (world)
    {
      world->ServerTravel(FString("/Game/ThirdPerson/Maps/Lobby?listen"));
    }
  }
  else
  {
    Debug::Print("FAILED TO CREATE SESSION!!", FColor::Red, 15.0f);
  }
}

void AMyNetworkPluginCharacter::OnFindSessionsComplete(bool bWasSuccessful)
{
  if (!OnlineSessionInterface.IsValid()) return;

  for (auto searchResult : SessionSearch->SearchResults)
  {
    FString id = searchResult.GetSessionIdStr();
    FString user = searchResult.Session.OwningUserName;
    FString matchType;
    searchResult.Session.SessionSettings.Get(FName("MatchType"), matchType);

    Debug::Print("ID: " + id + ", User: " + user, FColor::Cyan, 15.0f);

    if (matchType == FString("FreeForAll"))
    {
      // Join the session
      Debug::Print("Joining Match Type: " + matchType, FColor::Cyan, 15.0f);

      OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

      const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
      OnlineSessionInterface->JoinSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, searchResult);
    }
  }
}

void AMyNetworkPluginCharacter::OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result)
{
  if (!OnlineSessionInterface.IsValid()) return;

  FString address;
  if (OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, address))
  {
    Debug::Print("Connect string: " + address, FColor::Yellow, 15.0f);

    APlayerController* playerController = GetGameInstance()->GetFirstLocalPlayerController();
    if (playerController)
    {
      playerController->ClientTravel(address, ETravelType::TRAVEL_Absolute);
    }
  }
}
