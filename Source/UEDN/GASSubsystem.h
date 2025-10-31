#pragma once

#include "HttpFwd.h"
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Dom/JsonObject.h"
#include "Interfaces/IHttpResponse.h"
#include "GASSubsystem.generated.h"


//TODO:Is this code really needed?
USTRUCT(BlueprintType)
struct FGASRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FString uID;
	UPROPERTY(BlueprintReadOnly) FString Name;
	UPROPERTY(BlueprintReadOnly) int32 Rank = 0;
	UPROPERTY(BlueprintReadOnly) int Times = 0;
	UPROPERTY(BlueprintReadOnly) FString Timestamp;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGASFetchSuccess, const TArray<FGASRow>&, Rows);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGASFetchFail, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGASPostDone, bool, bSuccess, const FString&, Error);

UCLASS(Blueprintable)
class UEDN_API UGASSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable, Category="Demo")
	static void SayHello();

	// ---- setup / override at runtime ----
	UFUNCTION(BlueprintCallable, Category="UEDN")
	void ConfigureUEDN(const FString& InBaseUrl, const FString& InSecretKey);

	// ---- API: GET all rows ----
	UFUNCTION(BlueprintCallable, Category="UEDN")
	void FetchUEDN();

	// ---- API: POST append row ----
	UFUNCTION(BlueprintCallable, Category="UEDN")
	void PostUEDN(const FString& Uid, const FString& Name, int32 Rank, int32 Times);

	UPROPERTY(BlueprintAssignable, Category="UEDN")
	FGASFetchSuccess OnFetchSuccess;

	UPROPERTY(BlueprintAssignable, Category="UEDN")
	FGASFetchFail OnFetchFail;

	UPROPERTY(BlueprintAssignable, Category="UEDN")
	FGASPostDone OnPostDone;

private:
	FString BaseUrl;    // 例: https://script.google.com/.../exec
	FString SecretKey;  // HMAC 用

	// helpers
	static FString Sha256Base64(const FString& In);
	static FString HmacSha256Base64(const FString& Key, const FString& Msg);
	static FString ToJsonString(TSharedPtr<FJsonObject> Obj);

	void SendSignedPost(const FString& Path, TSharedPtr<FJsonObject> Data,
						TFunction<void(FHttpResponsePtr)> OnOK,
						TFunction<void(const FString&)> OnFail);

	void ParseFetchResponse(const FString& JsonStr);
};
