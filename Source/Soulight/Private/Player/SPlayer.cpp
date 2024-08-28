                                                            // Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SPlayer.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Player/SPlayerController.h"
#include "InputCoreTypes.h"
#include "GameFramework/PlayerInput.h"

#include "Animation/AnimInstance.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SceneComponent.h"

ASPlayer::ASPlayer()
{
	MainCameraPivot = CreateDefaultSubobject<USceneComponent>("Pivot of Main Camera");
	FullHealthView = CreateDefaultSubobject<USceneComponent>("Full Health Pos");
	EmptyHealthView = CreateDefaultSubobject<USceneComponent>("Empty Health Pos");
	MainCamera = CreateDefaultSubobject<UCameraComponent>("Main Camera");

	FullHealthView->SetupAttachment(GetRootComponent());
	FullHealthView->SetWorldLocation(FVector(-600, 0, 1000));
	FullHealthView->SetWorldRotation(FRotator(0, -60.0f, 0));

	EmptyHealthView->SetupAttachment(GetRootComponent());
	EmptyHealthView->SetWorldLocation(FVector(-300, 0, 400));
	EmptyHealthView->SetWorldRotation(FRotator(0, -60.0f, 0));

	MainCamera->SetupAttachment(MainCameraPivot, USpringArmComponent::SocketName);

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(1080.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
}

void ASPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MainCameraPivot->SetRelativeLocation(GetActorLocation());
}

void ASPlayer::BeginPlay()
{
	Super::BeginPlay();
}

void ASPlayer::PawnClientRestart()
{
	Super::PawnClientRestart();

	PlayerController = GetController<ASPlayerController>();
	if (PlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		InputSystem->ClearAllMappings();
		InputSystem->AddMappingContext(InputMapping, 0);
	}

}

void ASPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &ASPlayer::Move);
		EnhancedInputComponent->BindAction(AimInputAction, ETriggerEvent::Triggered, this, &ASPlayer::Aim);
		EnhancedInputComponent->BindAction(AttackInputAction, ETriggerEvent::Triggered, this, &ASPlayer::Attack);
		EnhancedInputComponent->BindAction(DodgeInputAction, ETriggerEvent::Triggered, this, &ASPlayer::Dodge);
		EnhancedInputComponent->BindAction(SkillInputAction, ETriggerEvent::Triggered, this, &ASPlayer::Skill);
		EnhancedInputComponent->BindAction(SpellInputAction, ETriggerEvent::Triggered, this, &ASPlayer::Spell);
		EnhancedInputComponent->BindAction(HUDInputAction, ETriggerEvent::Triggered, this, &ASPlayer::HUD);
		EnhancedInputComponent->BindAction(SettingsInputAction, ETriggerEvent::Triggered, this, &ASPlayer::Settings);
	}
}

void ASPlayer::Move(const FInputActionValue& InputValue)
{
	FVector2D Input = InputValue.Get<FVector2D>();
	Input.Normalize();

	if (previousDir != FVector(0.0f, 0.0f, 0.0f))
	{
		previousDir = FVector(0.0f, 0.0f, 0.0f);
	}
	else
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}

	AddMovementInput(Input.Y * GetMoveFwdDir() + Input.X * GetMoveRightDir());
}

void ASPlayer::Aim(const FInputActionValue& InputValue)
{
	FVector2D Input = InputValue.Get<FVector2D>();
	Input.Normalize();

	GetCharacterMovement()->bOrientRotationToMovement = false;

	previousDir = FVector(Input.X, Input.Y, 0.0f);

	SetActorRotation(FVector(Input.X, Input.Y, 0.0f).ToOrientationRotator());
}

void ASPlayer::Interact()
{

}

void ASPlayer::Attack()
{
	UE_LOG(LogTemp, Warning, TEXT("Attack"));
	HealthUpdated(1.0f);
}

void ASPlayer::Dodge()
{
	HealthUpdated(0.5f);
}

void ASPlayer::Skill()
{
	UE_LOG(LogTemp, Warning, TEXT("Skill"));
	HealthUpdated(0.0f);
}

void ASPlayer::Spell()
{
}

void ASPlayer::HUD()
{
}

void ASPlayer::Settings()
{
}

void ASPlayer::MoveCameraToLocalOffset(const FVector& LocalOffset)
{
	GetWorldTimerManager().ClearTimer(CameraTimerHandle);
	CameraTimerHandle = GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ASPlayer::ProcessCameraMove, LocalOffset));
}

void ASPlayer::ProcessCameraMove(FVector Goal)
{
	FVector CurrentLocalOffset = MainCamera->GetRelativeLocation();
	if (FVector::Dist(CurrentLocalOffset, Goal) < 1)
	{
		MainCamera->SetRelativeLocation(Goal);
		return;
	}

	FVector NewLocalOffset = FMath::Lerp(CurrentLocalOffset, Goal, GetWorld()->GetDeltaSeconds() * CameraMoveSpeed);
	MainCamera->SetRelativeLocation(NewLocalOffset);
	CameraTimerHandle = GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ASPlayer::ProcessCameraMove, Goal));
}

FVector ASPlayer::GetMoveFwdDir() const
{
	FVector CamerFwd = MainCamera->GetForwardVector();
	CamerFwd.Z = 0;
	return CamerFwd.GetSafeNormal();
}

FVector ASPlayer::GetMoveRightDir() const
{
	return MainCamera->GetRightVector();
}

void ASPlayer::HealthUpdated(const float newHealth)
{
	FVector interpolatedPos = FMath::Lerp(EmptyHealthView->GetRelativeLocation(), FullHealthView->GetRelativeLocation(), newHealth);
	MoveCameraToLocalOffset(interpolatedPos);
}
