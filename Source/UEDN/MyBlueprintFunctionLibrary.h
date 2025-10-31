// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UMyGameSubsystem.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UEDN_API UMyBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	public:
		UFUNCTION(BlueprintCallable, Category= "Demo Test")
		static FString text(FString Name);
	
        UFUNCTION(BlueprintCallable, Category = "Demo Test", meta = (WorldContext = "WorldContextObject"))
        static UUMyGameSubsystem* GetMyGameSubsystem(const UObject* WorldContextObject);
};
