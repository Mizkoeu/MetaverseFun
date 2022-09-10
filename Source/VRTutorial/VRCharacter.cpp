// Fill out your copyright notice in the Description page of Project Settings.

#include "VRCharacter.h"
#include "Curves/CurveFloat.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

// Sets default values
AVRCharacter::AVRCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetRootComponent());

	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Destination"));
	DestinationMarker->SetupAttachment(GetRootComponent());

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
	PostProcessComponent->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
	DestinationMarker->SetVisibility(false, true);
    LastHeadPosition = Camera->GetComponentLocation();

	if (BlinkerMaterialBase != nullptr)
	{
		BlinkerMaterialInstance = UMaterialInstanceDynamic::Create(BlinkerMaterialBase, this);
		PostProcessComponent->AddOrUpdateBlendable(BlinkerMaterialInstance);
		BlinkerMaterialInstance->SetScalarParameterValue(TEXT("Radius"), 1.0);
		BlinkerMaterialInstance->SetScalarParameterValue(TEXT("Density"), 5.0);
	}

	// Spawn hand controllers
	LeftController = GetWorld()->SpawnActor<AHandController>(HandController);
	if (LeftController != nullptr)
	{
		LeftController->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		LeftController->SetOwner(this);
		LeftController->SetHand(EControllerHand::Left);
        LeftController->AddTickPrerequisiteActor(this);
	}

	RightController = GetWorld()->SpawnActor<AHandController>(HandController);
	if (RightController != nullptr)
	{
		RightController->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		RightController->SetOwner(this);
		RightController->SetHand(EControllerHand::Right);
        RightController->AddTickPrerequisiteActor(this);
	}
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    ChangeInHeadHeight = abs((Camera->GetComponentLocation() - LastHeadPosition).Z);
    LastHeadPosition = Camera->GetComponentLocation();

    FVector HeadRelativePos = Camera->GetRelativeLocation();
    // Make sure player capsule follows the camera eyes.
    SetActorLocation(LastHeadPosition);
    Camera->AddWorldOffset(-HeadRelativePos);

    // Line trace to get which platform to stand on.
    FCollisionQueryParams params;
    params.AddIgnoredActor(this);
    FHitResult HitResult;
    FVector Downward(0.f, 0.f, -1.0);
    FVector HeadPos = Camera->GetComponentLocation();
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, HeadPos, HeadPos + Downward * 250, ECollisionChannel::ECC_Visibility, params);
    if (bHit && HitResult.Actor->ActorHasTag(TEXT("Platform")))
    {
        AttachToComponent(HitResult.GetComponent(), FAttachmentTransformRules::KeepWorldTransform, FName("StandPos"));
    }
    else
    {
        DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    }
    // Disable Teleport for now.
	// TickTeleport();
}

void AVRCharacter::TickTeleport()
{
    if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Flying)
    {
        DestinationMarker->SetVisibility(false, true);
        ResetMoveBlinker();
    }
    else
    {
        // Line trace for Teleportation marker.
        FCollisionQueryParams params;
        params.AddIgnoredComponent(DestinationMarker);
        params.AddIgnoredActor(this);
        FHitResult hit;
        FVector forward = RightController->GetActorForwardVector().RotateAngleAxis(50, RightController->GetActorRightVector());
        FVector pos = RightController->GetActorLocation();
        bool bHit = GetWorld()->LineTraceSingleByChannel(hit, pos, pos + forward * 1000, ECollisionChannel::ECC_Visibility, params);
        if (bHit)
        {
            DestinationMarker->SetVisibility(true, true);
            DestinationMarker->SetWorldLocation(hit.Location);
        }
        else
        {
            DestinationMarker->SetVisibility(false, true);
        }

        // To Prevent Motion Sickness.
        UpdateMoveBlinker();
    }
}

void AVRCharacter::UpdateMoveBlinker()
{
	float speed = GetVelocity().Size();
	if (BlinkerMaterialInstance != nullptr)
	{
		BlinkerMaterialInstance->SetScalarParameterValue(TEXT("Radius"), ShadowCurve->GetFloatValue(speed));
		BlinkerMaterialInstance->SetScalarParameterValue(TEXT("Density"), 3.33 - ShadowCurve->GetFloatValue(speed));
	}
}

void AVRCharacter::ResetMoveBlinker()
{
    BlinkerMaterialInstance->SetScalarParameterValue(TEXT("Radius"), 1.0);
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(TEXT("MovementAxisRight_Y"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MovementAxisRight_X"), this, &AVRCharacter::MoveRight);
	PlayerInputComponent->BindAction(TEXT("TriggerLeft"), IE_Released, this, &AVRCharacter::BeginTeleport);
	PlayerInputComponent->BindAction(TEXT("TriggerRight"), IE_Released, this, &AVRCharacter::BeginTeleport);
	PlayerInputComponent->BindAction(TEXT("GrabLeft"), IE_Pressed, this, &AVRCharacter::BeginGrabLeft);
	PlayerInputComponent->BindAction(TEXT("GrabRight"), IE_Pressed, this, &AVRCharacter::BeginGrabRight);
	PlayerInputComponent->BindAction(TEXT("GrabLeft"), IE_Released, this, &AVRCharacter::EndGrabLeft);
	PlayerInputComponent->BindAction(TEXT("GrabRight"), IE_Released, this, &AVRCharacter::EndGrabRight);
}

void AVRCharacter::BeginGrabLeft()
{
	LeftController->GrabTrigger();
}

void AVRCharacter::BeginGrabRight()
{
	RightController->GrabTrigger();
}

void AVRCharacter::EndGrabLeft()
{
	LeftController->GrabRelease();
}

void AVRCharacter::EndGrabRight()
{
	RightController->GrabRelease();
}

void AVRCharacter::BeginTeleport()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController != nullptr)
	{
		PlayerController->PlayerCameraManager->StartCameraFade(0, 1, TeleportFadeTime, FLinearColor::Black);
	}
	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, this, &AVRCharacter::EndTeleport, TeleportFadeTime);
}

void AVRCharacter::EndTeleport()
{
    float halfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	SetActorLocation(DestinationMarker->GetComponentLocation() + FVector(0, 0, halfHeight));	
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController != nullptr)
	{
		PlayerController->PlayerCameraManager->StartCameraFade(1, 0, TeleportFadeTime, FLinearColor::Black);
	}
}

void AVRCharacter::MoveForward(float speed)
{
	AddMovementInput(Camera->GetForwardVector(), speed);
}

void AVRCharacter::MoveRight(float speed)
{
	AddMovementInput(Camera->GetRightVector(), speed);	
}

void AVRCharacter::SetPrimaryHand(EControllerHand Hand)
{
    PrimaryHand = Hand;
    if (Hand == EControllerHand::Left)
    {
        RightController->bClimbing = false;
    }
    else
    {
        LeftController->bClimbing = false;
    }
}