// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBlueprintFunctionLibrary.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"

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

bool UMyBlueprintFunctionLibrary::ParseLeaderboardJson(const FString& Json, TArray<FUserRow>& OutRows)
{
	OutRows.Reset();

	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);

	TSharedPtr<FJsonObject> RootObject;
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseLeaderboardJson: Deserialize failed"));
		return false;
	}

	// Get the "data" array from the root object
	const TArray<TSharedPtr<FJsonValue>>* DataArray;
	if (!RootObject->TryGetArrayField(TEXT("data"), DataArray))
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseLeaderboardJson: 'data' field not found"));
		return false;
	}

	for (const TSharedPtr<FJsonValue>& Value : *DataArray)
	{
		if (!Value.IsValid() || Value->Type != EJson::Object)
		{
			continue;
		}

		TSharedPtr<FJsonObject> Obj = Value->AsObject();
		if (!Obj.IsValid())
		{
			continue;
		}

		FUserRow Row;

		Obj->TryGetStringField(TEXT("uID"), Row.UID);

		// Name can be number or string in your JSON
		FString NameString;
		if (Obj->TryGetStringField(TEXT("Name"), NameString))
		{
			Row.Name = NameString;
		}
		else
		{
			double NameNumber = 0.0;
			if (Obj->TryGetNumberField(TEXT("Name"), NameNumber))
			{
				Row.Name = FString::FromInt(static_cast<int32>(NameNumber));
			}
		}

		int32 Rank = 0;
		Obj->TryGetNumberField(TEXT("Rank"), Rank);
		Row.Rank = Rank;

		int32 Times = 0;
		Obj->TryGetNumberField(TEXT("Times"), Times);
		Row.Times = Times;

		// FString TimestampString;
		// if (Obj->TryGetStringField(TEXT("Timestamp"), TimestampString))
		// {
		// 	FDateTime::ParseIso8601(*TimestampString, Row.Timestamp);
		// }

		OutRows.Add(Row);
	}
	
	return true;
}