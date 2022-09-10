// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

UENUM(BlueprintType)
enum FGrabType
{
    None     UMETA(DisplayName = "None"),
    Free     UMETA(DisplayName = "Free"),
    Snap     UMETA(DisplayName = "Snap"),
    Custom   UMETA(DisplayName = "Custom"),
};