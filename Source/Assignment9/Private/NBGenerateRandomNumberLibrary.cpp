// Copyright Epic Games, Inc. All Rights Reserved.

#include "NBGenerateRandomNumberLibrary.h"

FString UNBGenerateRandomNumberLibrary::GenerateRandomNumber()
{
	FString GoalNumber = "/";
	int32 Temp1 = FMath::RandRange(1, 9);
	int32 Temp2 = FMath::RandRange(1, 9);
	int32 Temp3 = FMath::RandRange(1, 9);
	while (Temp1 == Temp2 || Temp2 == Temp3 || Temp3 == Temp1)
	{
		Temp1 = FMath::RandRange(1, 9);
		Temp2 = FMath::RandRange(1, 9);
		Temp3 = FMath::RandRange(1, 9);
	}

	GoalNumber += FString::FromInt(Temp1);
	GoalNumber += FString::FromInt(Temp2);
	GoalNumber += FString::FromInt(Temp3);

	return GoalNumber;
}