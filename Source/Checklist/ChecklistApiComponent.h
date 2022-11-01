// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ChecklistApiComponent.generated.h"

class IWebSocket;
USTRUCT(BlueprintType)
struct FChecklistItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = TEXT("Unknown");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bState = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUpdateList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FConnected);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHECKLIST_API UChecklistApiComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UChecklistApiComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	TSharedPtr<IWebSocket> Socket;

	void ReceiveUpdateList(const TArray<TSharedPtr<FJsonValue>>& ListJson);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FChecklistItem> List;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SocketUrl = TEXT("ws://localhost:3000");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ServerProtocol = TEXT("ws");
	
	UPROPERTY(BlueprintAssignable)
	FUpdateList OnUpdateList;

	UPROPERTY(BlueprintAssignable)
	FConnected OnConnected;

	UFUNCTION(BlueprintCallable)
	void InsertItem(const FChecklistItem NewItem);

	UFUNCTION(BlueprintCallable)
	void RemoveItem(const int Id);

	UFUNCTION(BlueprintCallable)
	void ToggleItem(const int Id, const bool bNewState);

};
