#include "GASSubsystem.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Misc/ConfigCacheIni.h"

void UGASSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadConfig();
    
    // 設定が読み込まれたことをログに出力
    UE_LOG(LogTemp, Log, TEXT("GASSubsystem initialized"));
    UE_LOG(LogTemp, Log, TEXT("WebAppURL: %s"), *WebAppURL);
    UE_LOG(LogTemp, Log, TEXT("SecretKey: %s"), SecretKey.IsEmpty() ? TEXT("[Empty]") : *SecretKey);
}

void UGASSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UGASSubsystem::LoadConfig()
{
    FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("Endpoints.Local.ini");
    
    if (GConfig)
    {
        FString WebAppURLPath;

        GConfig->GetString(
            TEXT("/Script/UEDN.GASSubsystem"),
            TEXT("WebAppURLPath"), 
            WebAppURLPath,
            ConfigPath
        );

        UE_LOG(LogTemp, Log, TEXT("WebAppURLPath: %s"), *WebAppURLPath);

        if (!WebAppURLPath.StartsWith(TEXT("https://")))
        {
            WebAppURL = FString::Printf(TEXT("https://%s"), *WebAppURLPath);
        }
        else
        {
            WebAppURL = WebAppURLPath;
        }
        
        GConfig->GetString(
            TEXT("/Script/UEDN.GASSubsystem"),
            TEXT("SecretKey"),
            SecretKey,
            ConfigPath
        );
    }
}

void UGASSubsystem::WriteToSheet(const FString& UID, const FString& Name, const FString& Rank, int32 Times)
{
    if (WebAppURL.IsEmpty() || SecretKey.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("WebAppURL or SecretKey is not set in Endpoints.Local.ini"));
        OnSheetResponse.Broadcast(false, TEXT("Configuration error"));
        return;
    }

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("key"), SecretKey);
    JsonObject->SetStringField(TEXT("operation"), TEXT("write"));
    JsonObject->SetStringField(TEXT("uID"), UID);
    JsonObject->SetStringField(TEXT("Name"), Name);
    JsonObject->SetStringField(TEXT("Rank"), Rank);
    JsonObject->SetNumberField(TEXT("Times"), Times);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    SendRequest(OutputString);
}

void UGASSubsystem::ReadFromSheet()
{
    if (WebAppURL.IsEmpty() || SecretKey.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("WebAppURL or SecretKey is not set in Endpoints.Local.ini"));
        OnSheetResponse.Broadcast(false, TEXT("Configuration error"));
        return;
    }

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("key"), SecretKey);
    JsonObject->SetStringField(TEXT("operation"), TEXT("read"));

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    SendRequest(OutputString);
}

void UGASSubsystem::SendRequest(const FString& JsonPayload)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(WebAppURL);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetContentAsString(JsonPayload);
    Request->OnProcessRequestComplete().BindUObject(this, &UGASSubsystem::OnResponseReceived);
    Request->ProcessRequest();
}

void UGASSubsystem::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("HTTP Request failed"));
        OnSheetResponse.Broadcast(false, TEXT("Request failed"));
        return;
    }

    FString ResponseString = Response->GetContentAsString();
    UE_LOG(LogTemp, Log, TEXT("Response: %s"), *ResponseString);
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);
    
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        bool bSuccess = JsonObject->GetBoolField(TEXT("success"));
        OnSheetResponse.Broadcast(bSuccess, ResponseString);
    }
    else
    {
        OnSheetResponse.Broadcast(false, TEXT("Failed to parse response"));
    }
}