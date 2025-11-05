// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBlueprintFunctionLibrary.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"


FString UMyBlueprintFunctionLibrary::text(FString Name)
{
	return FString();
}


UUMyGameSubsystem* UMyBlueprintFunctionLibrary::GetMyGameSubsystem(const UObject* WorldContextObject)
{
	if (UGameInstance* GameInstance = WorldContextObject->GetWorld()->GetGameInstance())
	{
   		return GameInstance->GetSubsystem<UUMyGameSubsystem>();
	}
	return nullptr;
}

//UGASSubsystem* UMyBlueprintFunctionLibrary::GetGASSubsystem(const UObject* WorldContextObject)
//{
//	if (UGameInstance* GameInstance = WorldContextObject->GetWorld()->GetGameInstance())
//	{
//		return GameInstance->GetSubsystem<UGASSubsystem>();
//	}
//	return nullptr;
//}
