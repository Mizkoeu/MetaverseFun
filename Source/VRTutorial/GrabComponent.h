// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "MotionControllerComponent.h"
#include "GrabType.h"
#include "GrabComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class VRTUTORIAL_API UGrabComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrabComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // UPROPERTY(EditAnywhere, BlueprintReadOnly)
    // struct GrabType GrabeType;
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    bool TryGrab(UMotionControllerComponent* MotionController);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    bool TryRelease();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHeld;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSimulateOnDrop;		
};
