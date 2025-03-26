// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NBPlayerState.generated.h"

/**
 *
 */
UCLASS()
class ASSIGNMENT9_API ANBPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ANBPlayerState();

private:
	FString UserID;
	FString Result;
public:
	int32 DefaultPlayerChance;

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	FString GetUserID() const { return UserID; }
	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	bool GetIsMyTurn() const { return bIsMyTurn; }
	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	int32 GetPlayerScore() const { return PlayerScore; }
	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	int32& GetPlayerChance() { return PlayerChance; }

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void SetUserID(const FString& InUserID) { UserID = InUserID; }
	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void SetIsMyTurn(bool bInIsMyTurn) { bIsMyTurn = bInIsMyTurn; }
	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void SetPlayerScore(int32 InPlayerScore) { PlayerScore = InPlayerScore; }
	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void SetPlayerChance(int32 InPlayerChance) { PlayerChance = InPlayerChance; }

public:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerChance)
	int32 PlayerScore;
	UPROPERTY(ReplicatedUsing = OnRep_PlayerChance)
	int32 PlayerChance;
	UPROPERTY(ReplicatedUsing = OnRep_IsMyTurn)
	bool bIsMyTurn;

	UFUNCTION()
	void OnRep_IsMyTurn();
	UFUNCTION()
	void OnRep_PlayerScore();
	UFUNCTION()
	void OnRep_PlayerChance();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
