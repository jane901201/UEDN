// ELanguage.h
#pragma once

#include "CoreMinimal.h"
#include "ELanguage.generated.h"

UENUM(BlueprintType)
enum class ELanguage : uint8
{
	TraditionalChinese  UMETA(DisplayName = "繁體中文"),
	SimplifiedChinese   UMETA(DisplayName = "简体中文"),
	English             UMETA(DisplayName = "English"),
	Japanese            UMETA(DisplayName = "日本語")
};