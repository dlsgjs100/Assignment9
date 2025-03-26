// Copyright Epic Games, Inc. All Rights Reserved.

#include "NBGameRulesLibrary.h"

void UNBGameRulesLibrary::DecreaseChances(int& PlayerChances)
{
	if (PlayerChances > 0)
	{
		PlayerChances--;
	}
}

bool UNBGameRulesLibrary::IsValidResponse(const FString& Response)
{
	FRegexPattern Pattern(TEXT("^/([1-9])(?!.*\\1)([1-9])(?!.*\\2)([1-9])$"));
	FRegexMatcher Matcher(Pattern, Response);
	return Matcher.FindNext();
}

FString UNBGameRulesLibrary::CheckStrikeBall(const FString& Response, const FString& GoalNumber)
{
	if (!IsValidResponse(Response))
	{
		return FString("Invalid Response");
	}

	int32 StrikeCount = 0;
	int32 BallCount = 0;
	int32 GoalNumberLength = GoalNumber.Len();
	for (int32 i = 1; i < GoalNumberLength; i++)
	{
		if (GoalNumber[i] == Response[i])
		{
			StrikeCount++;
		}

		for (int32 j = 1; j < GoalNumberLength; j++)
		{
			if (i != j && GoalNumber[i] == Response[j])
			{
				BallCount++;
			}
		}
	}

	FString Result;

	if (StrikeCount == 0 && BallCount == 0)
	{
		Result = FString::Printf(TEXT("OUT"));
	}
	else
	{
		Result = FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
	}
	return Result;
}

void UNBGameRulesLibrary::WinGame(const FString& UserID, bool& bGameOver)
{
	FString WinMessage = FString::Printf(TEXT("%s Won!!"), *UserID);
	bGameOver = true;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, WinMessage);
}

void UNBGameRulesLibrary::DefeatGame(const FString& UserID, bool& bGameOver)
{
	FString DefeatMessage = FString::Printf(TEXT("%s Defeat!!"), *UserID);
	bGameOver = true;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, DefeatMessage);
}

void UNBGameRulesLibrary::DrawGame(bool& bGameOver)
{
	FString DrawMessage = TEXT("Draw!!");
	bGameOver = true;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, DrawMessage);
}

void UNBGameRulesLibrary::ResetGame(int& PlayerChances, bool& bGameOver)
{
	FString ResetMessage = TEXT("Game Reset!!");
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, ResetMessage);
	PlayerChances = 3;
	bGameOver = false;
}
