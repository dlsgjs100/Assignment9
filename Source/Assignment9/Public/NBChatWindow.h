// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NBChatWindow.generated.h"

/**
 *
 */
class UCanvasPanel;
class UEditableTextBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUserInputCommitted, const FString&, UserInputText);
UCLASS()
class ASSIGNMENT9_API UNBChatWindow : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* CanvasPanel;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* UserInputTextBox;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUserInputCommitted OnUserInputCommitted;

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnUserInputTextCommitted(const FText& Text, ETextCommit::Type CommitType);
};
