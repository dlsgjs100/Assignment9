// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NBGameRulesLibrary.generated.h"

/**
 *
 */
UCLASS()
class ASSIGNMENT9_API UNBGameRulesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Game Rules")
	static void DecreaseChances(int& PlayerChances);

	UFUNCTION(BlueprintCallable, Category = "Game Rules")
	static bool IsValidResponse(const FString& Response);

	UFUNCTION(BlueprintCallable, Category = "Game Rules")
	static FString CheckStrikeBall(const FString& Response, const FString& GoalNumber);

	UFUNCTION(BlueprintCallable, Category = "Game Rules")
	static void WinGame(const FString& UserID, bool& bGameOver);

	UFUNCTION(BlueprintCallable, Category = "Game Rules")
	static void DefeatGame(const FString& UserID, bool& bGameOver);

	UFUNCTION(BlueprintCallable, Category = "Game Rules")
	static void DrawGame(bool& bGameOver);

	UFUNCTION(BlueprintCallable, Category = "Game Rules")
	static void ResetGame(int& PlayerChances, bool& bGameOver);
};