// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GrabComponent.h"
#include "GameFramework/Actor.h"
#include "MotionControllerComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "HandController.generated.h"

UCLASS()
class VRTUTORIAL_API AHandController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHandController();

	bool bClimbing = false;
    bool bWalking = false;
	void GrabTrigger();
	void GrabRelease();
    
	void SetHand(EControllerHand Hand);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void ActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

    void GetGrabComponentNearController();

    void ReleaseHeldComponent();

    float GrabRadius = 6.0;
    
    // Helper
	bool CanClimb() const;
	bool bCanClimb = false;
	EControllerHand HandSide;

	FVector LastKnownPosition;

    UPROPERTY(EditDefaultsOnly)
	UGrabComponent* HeldComponent;
	
	UPROPERTY()
	UMotionControllerComponent* MotionController;

	UPROPERTY(EditDefaultsOnly)
	UHapticFeedbackEffect_Base* HapticFeedback;
};
