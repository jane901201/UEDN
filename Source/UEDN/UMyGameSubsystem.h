// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UMyGameSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class UEDN_API UUMyGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	public:
        // サブシステムの初期化
        virtual void Initialize(FSubsystemCollectionBase& Collection) override;
        
        // サブシステムの終了処理
        virtual void Deinitialize() override;
    
        // Blueprintから呼び出せる関数
        UFUNCTION(BlueprintCallable, Category = "Game")
        void SavePlayerData(int32 Score);
    
        UFUNCTION(BlueprintPure, Category = "Game")
        int32 GetPlayerScore() const;
    
    protected:
        UPROPERTY(BlueprintReadOnly, Category = "Game")
        int32 PlayerScore = 0;
	
};
