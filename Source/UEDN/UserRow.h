// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UserRow.generated.h"

USTRUCT(BlueprintType)
struct FUserRow
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Score")
	FString UID;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Score")
	FString Name;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Score")
	int32 Rank = 0;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Score")
	int32 Times = 0;
};
