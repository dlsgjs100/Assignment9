// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NBGenerateRandomNumberLibrary.generated.h"

/**
 *
 */
UCLASS()
class ASSIGNMENT9_API UNBGenerateRandomNumberLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "GenerateRandomNumber")
	static FString GenerateRandomNumber();
};
