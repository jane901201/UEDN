// Fill out your copyright notice in the Description page of Project Settings.


#include "UMyGameSubsystem.h"

void UUMyGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("MyGameSubsystem Initialized"));
    
    // 初期化処理
    PlayerScore = 0;
}

void UUMyGameSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("MyGameSubsystem Deinitialized"));
    Super::Deinitialize();
}

void UUMyGameSubsystem::SavePlayerData(int32 Score)
{
    PlayerScore = Score;
    UE_LOG(LogTemp, Log, TEXT("Player Score Saved: %d"), PlayerScore);
}

int32 UUMyGameSubsystem::GetPlayerScore() const
{
    return PlayerScore;
}