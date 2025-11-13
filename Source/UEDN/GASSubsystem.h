#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "GASSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSheetResponse, bool, bSuccess, const FString&, ResponseData);

UCLASS()
class UEDN_API UGASSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Google Sheets")
    FString WebAppURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Google Sheets")
    FString SecretKey;

    UPROPERTY(BlueprintAssignable, Category = "Google Sheets")
    FOnSheetResponse OnSheetResponse;

    // シートにデータを書き込む
    UFUNCTION(BlueprintCallable, Category = "Google Sheets")
    void WriteToSheet(const FString& UID, const FString& Name, const FString& Rank, int32 Times);

    // シートからデータを読み取る
    UFUNCTION(BlueprintCallable, Category = "Google Sheets")
    void ReadFromSheet();

private:
    void SendRequest(const FString& JsonPayload);
    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void LoadConfig();
};