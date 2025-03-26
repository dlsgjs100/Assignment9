// Copyright Epic Games, Inc. All Rights Reserved.

#include "NBPlayerState.h"
#include "NBPlayerController.h"
#include "NBGameState.h"
#include "Net/UnrealNetwork.h"

ANBPlayerState::ANBPlayerState()
{
	UserID = TEXT("");
	bIsMyTurn = false;
	PlayerScore = 0;
	DefaultPlayerChance = 3;
}

void ANBPlayerState::OnRep_IsMyTurn()
{
    ANBPlayerController* NBPlayerController = Cast<ANBPlayerController>(GetPlayerController());
    if (NBPlayerController)
    {
        NBPlayerController->ClientUpdateChatWindowWidget();
    }
}

void ANBPlayerState::OnRep_PlayerScore()
{
	UE_LOG(LogTemp, Warning, TEXT("PlayerScore Set : %d"), PlayerScore);
}

void ANBPlayerState::OnRep_PlayerChance()
{
	UE_LOG(LogTemp, Warning, TEXT("PlayerChance Set : %d"), PlayerChance);
}

void ANBPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANBPlayerState, bIsMyTurn);
	DOREPLIFETIME(ANBPlayerState, PlayerChance);
	DOREPLIFETIME(ANBPlayerState, PlayerScore);
}