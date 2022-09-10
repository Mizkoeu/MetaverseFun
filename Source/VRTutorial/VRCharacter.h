// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Math/Vector.h"
#include "HandController.h"
#include "GrabComponent.h"
#include "VRCharacter.generated.h"

UCLASS()
class VRTUTORIAL_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float speed);
	void MoveRight(float speed);
    void SetPrimaryHand(EControllerHand Hand);
    EControllerHand GetPrimaryHand() { return PrimaryHand; }
    float GetHeadHeightChange() { return ChangeInHeadHeight > HeadMovementThreshold ? ChangeInHeadHeight : 0; };

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UCameraComponent* Camera;

private:
    void TickTeleport();
	void BeginTeleport();
	void EndTeleport();
	void UpdateMoveBlinker();
    void ResetMoveBlinker();
	void BeginGrabLeft();
	void BeginGrabRight();
	void EndGrabLeft();
	void EndGrabRight();

    EControllerHand PrimaryHand;

    FVector LastHeadPosition;
    float ChangeInHeadHeight;

    UPROPERTY(EditDefaultsOnly)
	float HeadMovementThreshold = 0.1;

	UPROPERTY(EditAnywhere)
	float TeleportFadeTime = 0.5;

	UPROPERTY(EditDefaultsOnly)
	class UMaterialInterface* BlinkerMaterialBase;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AHandController> HandController;

    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UGrabComponent> GrabComponent;

	UPROPERTY(EditDefaultsOnly)
	class UCurveFloat* ShadowCurve;

	UPROPERTY()
	class AHandController* LeftController;

	UPROPERTY()
	class AHandController* RightController;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* DestinationMarker;
	
	UPROPERTY()
	class UPostProcessComponent* PostProcessComponent;

	UPROPERTY()
	class UMaterialInstanceDynamic* BlinkerMaterialInstance;
};
