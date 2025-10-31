#include "GASSubsystem.h"
#include "Engine/Engine.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"
#include "Misc/Base64.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Guid.h"
#include "CoreMinimal.h"
#include "JsonObjectConverter.h"

#define UI UI_ST
THIRD_PARTY_INCLUDES_START
#include "openssl/evp.h"
#include "openssl/hmac.h"
THIRD_PARTY_INCLUDES_END
#undef UI

void UGASSubsystem::SayHello()
{
	UE_LOG(LogTemp, Warning, TEXT("Hello from GameInstanceSubsystem!"));
}

// ================== Utilities ==================
FString UGASSubsystem::Sha256Base64(const FString& In)
{
    FTCHARToUTF8 Conv(*In);
    uint8 Hash[32];
    TArray<uint8> Bytes; Bytes.Append(Hash, 32);
    return FBase64::Encode(Bytes);
}

FString UGASSubsystem::HmacSha256Base64(const FString& Key, const FString& Msg)
{
    FTCHARToUTF8 K(*Key);
    FTCHARToUTF8 M(*Msg);
    unsigned int Len = EVP_MAX_MD_SIZE;
    unsigned char Out[EVP_MAX_MD_SIZE] = {0};

    HMAC(EVP_sha256(),
         (const unsigned char*)K.Get(), K.Length(),
         (const unsigned char*)M.Get(), M.Length(),
         Out, &Len);

    TArray<uint8> Sig; Sig.Append(Out, (int32)Len);
    return FBase64::Encode(Sig);
}

FString UGASSubsystem::ToJsonString(TSharedPtr<FJsonObject> Obj)
{
    FString Out;
    TSharedRef<TJsonWriter<>> W = TJsonWriterFactory<>::Create(&Out);
    FJsonSerializer::Serialize(Obj.ToSharedRef(), W);
    return Out;
}

// ================== Subsystem ==================
void UGASSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    if (GConfig)
    {
        FString LoadedUrl, LoadedKey;
        // ファイルを事前に Load する場合：
        // GConfig->LoadFile(TEXT("Config/Endpoints.Local.ini"));
        GConfig->GetString(TEXT("UEDN"), TEXT("BaseUrl"), LoadedUrl, GGameIni);
        GConfig->GetString(TEXT("UEDN"), TEXT("SecretKey"), LoadedKey, GGameIni);

        if (!LoadedUrl.IsEmpty())  BaseUrl = LoadedUrl;
        if (!LoadedKey.IsEmpty())  SecretKey = LoadedKey;
    }
}

void UGASSubsystem::Deinitialize()
{
    // nothing for now
}

void UGASSubsystem::ConfigureUEDN(const FString& InBaseUrl, const FString& InSecretKey)
{
    if (!InBaseUrl.IsEmpty()) BaseUrl = InBaseUrl;
    if (!InSecretKey.IsEmpty()) SecretKey = InSecretKey;
}

// ================== Core send (HMAC body-meta) ==================
void UGASSubsystem::SendSignedPost(const FString& Path,
                                    TSharedPtr<FJsonObject> Data,
                                    TFunction<void(FHttpResponsePtr)> OnOK,
                                    TFunction<void(const FString&)> OnFail)
{
    if (BaseUrl.IsEmpty() || SecretKey.IsEmpty())
    {
        OnFail(TEXT("Not configured: BaseUrl or SecretKey is empty"));
        return;
    }

    // 1) data JSON
    if (!Data.IsValid()) Data = MakeShared<FJsonObject>();
    const FString DataJson = ToJsonString(Data);

    // 2) body hash
    const FString BodyHash = Sha256Base64(DataJson);

    // 3) meta fields
    const FString Method = TEXT("POST");
    const FString CanonPath = Path;     // 例: "/exec"
    const FString Ts    = FString::Printf(TEXT("%lld"), FDateTime::UtcNow().ToUnixTimestamp());
    const FString Nonce = FGuid::NewGuid().ToString(EGuidFormats::Digits);

    // 4) canonical + HMAC
    const FString Canonical = Method + TEXT("\n") + CanonPath + TEXT("\n") + Ts + TEXT("\n") + Nonce + TEXT("\n") + BodyHash;
    const FString Sig = HmacSha256Base64(SecretKey, Canonical);

    // 5) payload = { meta, data }
    TSharedPtr<FJsonObject> Meta = MakeShared<FJsonObject>();
    Meta->SetStringField(TEXT("ts"), Ts);
    Meta->SetStringField(TEXT("nonce"), Nonce);
    Meta->SetStringField(TEXT("sig"), Sig);
    Meta->SetStringField(TEXT("method"), Method);
    Meta->SetStringField(TEXT("path"), CanonPath);

    TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
    Payload->SetObjectField(TEXT("meta"), Meta);
    Payload->SetObjectField(TEXT("data"), Data);

    const FString PayloadJson = ToJsonString(Payload);

    // 6) HTTP POST
    auto& Http = FHttpModule::Get();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = Http.CreateRequest();
    Req->SetURL(BaseUrl + CanonPath);           
    Req->SetVerb(TEXT("POST"));
    Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Req->SetContentAsString(PayloadJson);

    Req->OnProcessRequestComplete().BindLambda(
        [OnOK, OnFail](FHttpRequestPtr, FHttpResponsePtr Resp, bool bOK)
        {
            if (!bOK || !Resp.IsValid())
            {
                OnFail(TEXT("Network failure"));
                return;
            }
            if (Resp->GetResponseCode() >= 200 && Resp->GetResponseCode() < 300)
            {
                OnOK(Resp);
            }
            else
            {
                OnFail(FString::Printf(TEXT("HTTP %d: %s"),
                      Resp->GetResponseCode(), *Resp->GetContentAsString()));
            }
        });

    Req->ProcessRequest();
}

// ================== Public API ==================
void UGASSubsystem::FetchUEDN()
{
    // 署名付き GET に合わせるため、POST+空 data で doGet 相当のルートを Apps Script 側に用意するか、
    // ここでは簡便に「/exec」を doPost 側で READ/WRITE 兼用ハンドリングする設計を想定します。
    TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("_op"), TEXT("read"));

    SendSignedPost(TEXT("/exec"), Data,
        [this](FHttpResponsePtr Resp)
        {
            const FString Body = Resp->GetContentAsString();
            ParseFetchResponse(Body);
        },
        [this](const FString& Reason)
        {
            OnFetchFail.Broadcast(Reason);
        });
}

void UGASSubsystem::ParseFetchResponse(const FString& JsonStr)
{
    TSharedPtr<FJsonObject> Root;
    const TSharedRef<TJsonReader<>> R = TJsonReaderFactory<>::Create(JsonStr);
    if (!FJsonSerializer::Deserialize(R, Root) || !Root.IsValid())
    {
        OnFetchFail.Broadcast(TEXT("Bad JSON"));
        return;
    }

    const TArray<TSharedPtr<FJsonValue>>* RowsJson = nullptr;
    if (!Root->TryGetArrayField(TEXT("rows"), RowsJson))
    {
        OnFetchFail.Broadcast(TEXT("No rows field"));
        return;
    }

    TArray<FGASRow> Rows;
    for (const auto& V : *RowsJson)
    {
        const TSharedPtr<FJsonObject> O = V->AsObject();
        if (!O.IsValid()) continue;

        FGASRow Row;
        Row.uID      = O->GetStringField(TEXT("uID"));
        Row.Name     = O->GetStringField(TEXT("Name"));
        Row.Rank     = (int32)O->GetNumberField(TEXT("Rank"));
        Row.Times    = (int32)O->GetNumberField(TEXT("Times"));
        Row.Timestamp= O->GetStringField(TEXT("Timestamp"));
        Rows.Add(Row);
    }
    OnFetchSuccess.Broadcast(Rows);
}

void UGASSubsystem::PostUEDN(const FString& Uid, const FString& Name, int32 Rank, int32 Times)
{
    TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
    Data->SetStringField(TEXT("_op"), TEXT("append"));
    Data->SetStringField(TEXT("uID"), Uid);
    Data->SetStringField(TEXT("Name"), Name);
    Data->SetNumberField(TEXT("Rank"), Rank);
    Data->SetNumberField(TEXT("Times"), Times);

    SendSignedPost(TEXT("/exec"), Data,
        [this](FHttpResponsePtr Resp)
        {
            OnPostDone.Broadcast(true, Resp->GetContentAsString());
        },
        [this](const FString& Reason)
        {
            OnPostDone.Broadcast(false, Reason);
        });
}