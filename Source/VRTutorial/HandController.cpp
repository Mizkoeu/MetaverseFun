// Fill out your copyright notice in the Description page of Project Settings.

#include "HandController.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Components/SceneComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "VRCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AHandController::AHandController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController"));
	MotionController->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AHandController::BeginPlay()
{
	Super::BeginPlay();
	OnActorBeginOverlap.AddDynamic(this, &AHandController::ActorBeginOverlap);
	OnActorEndOverlap.AddDynamic(this, &AHandController::ActorEndOverlap);
}

void AHandController::SetHand(EControllerHand Hand)
{
	MotionController->SetTrackingSource(Hand);
	HandSide = Hand;
    UE_LOG(LogTemp, Warning, TEXT("Hello I am a hand controller %s"), HandSide == EControllerHand::Left ? TEXT("LEFT") : TEXT("RIGHT"));

	// Set right hand with proper orientation
	FTransform RightMirrorTransform 
	{
		FRotator(0, -40, 0),
		FVector(0, 0, 0),
		FVector(1, -1, 1)
	};

	if (HandSide == EControllerHand::Right)
	{
		MotionController->GetChildComponent(0)->SetRelativeTransform(RightMirrorTransform);
	}
}

void AHandController::ActorBeginOverlap(AActor* Actor, AActor* OtherActor)
{
	bool bNewCanClimb = CanClimb();
	if (bNewCanClimb && !bCanClimb)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hello, I'm climbing wow!"));
		// Play haptic!
		APlayerController* Controller = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
		Controller->PlayHapticEffect(HapticFeedback, HandSide, 1.5, false);
	}
	bCanClimb = bNewCanClimb;
}

void AHandController::ActorEndOverlap(AActor* Actor, AActor* OtherActor)
{
	bCanClimb = CanClimb();
}

bool AHandController::CanClimb() const
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors);
	for (AActor* Actor : OverlappingActors)
	{
		if (Actor->ActorHasTag(TEXT("Climbable")))
		{
			return true;
		}
	}
	return false;
}

void AHandController::GrabTrigger()
{
    GetGrabComponentNearController();

	//bClimbing = bCanClimb;
	if (!bCanClimb) 
    {
        // Let's start walking here!
        if (!bWalking)
        {
            bWalking = true;
            // Update last frame location of HandController.
            LastKnownPosition = GetActorLocation() - GetAttachParentActor()->GetActorLocation();
        }
        return;
    }

	if (!bClimbing)
    {
        bClimbing = true;
        // Update last frame location of HandController.
        LastKnownPosition = GetActorLocation();
        AVRCharacter* Character = Cast<AVRCharacter>(GetAttachParentActor());
        if (Character != nullptr)
        {
            Character->SetPrimaryHand(HandSide);
            Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
        }
	}
}

void AHandController::GrabRelease()
{
    ReleaseHeldComponent();

	// Throw myself off
    if (bClimbing)
    {
        bClimbing = false;
        ACharacter* Character = Cast<ACharacter>(GetAttachParentActor());
        if (Character != nullptr)
        {
            Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
            UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Character->GetComponentByClass(UStaticMeshComponent::StaticClass()));
            if (Mesh != nullptr)
            {
                FVector ChangeInPosition = GetActorLocation() - LastKnownPosition;
                Mesh->SetSimulatePhysics(true);
                Mesh->AddImpulse(-ChangeInPosition * 1000000 * 120);
            }
        }
    }
    
    if (bWalking)
    {
        // Stop walking logic now!
        bWalking = false;
    }
}

void AHandController::GetGrabComponentNearController()
{
    FVector GripLocation = MotionController->GetComponentLocation();
    FHitResult HitResult;
    TArray<AActor*> IgnoreActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypeQuery;
    ObjectTypeQuery.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
    bool bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), GripLocation, GripLocation, GrabRadius, ObjectTypeQuery, false, IgnoreActors, EDrawDebugTrace::None, HitResult, true);
    if (bHit)
    {
        UGrabComponent* Grabbed = HitResult.GetActor()->FindComponentByClass<UGrabComponent>();
        if (Grabbed != nullptr)
        {
            if (Grabbed->TryGrab(MotionController))
            {
                HeldComponent = Grabbed;
            }
        }
    }
}

void AHandController::ReleaseHeldComponent()
{
    if (HeldComponent != nullptr)
    {
        if (HeldComponent->TryRelease())
        {
            HeldComponent = nullptr;
        }
    }
}

// Called every frame
void AHandController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    AVRCharacter* Character = Cast<AVRCharacter>(GetAttachParentActor());
    if (Character != nullptr)
    {
        if (bClimbing && Character->GetPrimaryHand() == HandSide)
        {
            FVector ChangeInPosition = GetActorLocation() - LastKnownPosition;
            Character->AddActorWorldOffset(-ChangeInPosition);
        }
        else if (bWalking)
        {
            FVector CurrentRelativeLocation = GetActorLocation() - GetAttachParentActor()->GetActorLocation();
            float ChangeInPosition = (CurrentRelativeLocation - LastKnownPosition).Size();
            float HeadHeightChange = Character->GetHeadHeightChange();
            Character->MoveForward(ChangeInPosition * HeadHeightChange/(DeltaTime * 50));
            LastKnownPosition = CurrentRelativeLocation;
        }
    }
}

