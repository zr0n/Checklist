// Fill out your copyright notice in the Description page of Project Settings.


#include "ChecklistApiComponent.h"

#include "IContentBrowserSingleton.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h" 
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
UChecklistApiComponent::UChecklistApiComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	Socket = FWebSocketsModule::Get().CreateWebSocket(SocketUrl, ServerProtocol);

	Socket->OnConnected().AddLambda([&]() -> void
	{
		OnConnected.Broadcast();
	});

	Socket->OnMessage().AddLambda([&](const FString& Message) -> void
	{
		TSharedPtr<FJsonValue> ContentJson;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

		if (FJsonSerializer::Deserialize(Reader, ContentJson))
		{
			const FString Type = ContentJson->AsObject()->GetStringField("type");
			

			if(Type.Equals(TEXT("list")))
			{
				ReceiveUpdateList(ContentJson->AsObject()->GetArrayField("list"));
			}
			else
			{
				UKismetSystemLibrary::PrintString(this, TEXT("Unknown command received from server."));
				UKismetSystemLibrary::PrintString(this, Message);
			}
		}
	});

	// ...
}


// Called when the game starts
void UChecklistApiComponent::BeginPlay()
{
	Super::BeginPlay();

	List = TArray<FChecklistItem>();
	
	Socket->Connect();

	// ...
	
}


void UChecklistApiComponent::ReceiveUpdateList(const TArray<TSharedPtr<FJsonValue>>& ListJson)
{
	if(List.Num())
	{
		List.Empty();
	}
	for(const auto Item : ListJson)
	{
		FChecklistItem ThisItem;
		const auto ItemObj = Item->AsObject();
		ThisItem.Id = ItemObj->GetNumberField("id");
		ThisItem.Name = ItemObj->GetStringField("name");
		ThisItem.bState = ItemObj->GetBoolField("state");

		List.Add(ThisItem);
	}

	OnUpdateList.Broadcast();
}

// Called every frame
void UChecklistApiComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UChecklistApiComponent::InsertItem(const FChecklistItem NewItem)
{
	const TSharedPtr<FJsonObject> ItemJson = MakeShareable(new FJsonObject);
	const TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("type"), TEXT("insert"));

	ItemJson->SetNumberField(TEXT("id"), NewItem.Id);
	ItemJson->SetStringField(TEXT("name"), NewItem.Name);
	ItemJson->SetBoolField(TEXT("state"), NewItem.bState);

	JsonObject->SetObjectField(TEXT("item"), ItemJson);

	FString OutputString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	Socket->Send(OutputString);
}

void UChecklistApiComponent::RemoveItem(const int Id)
{
	const TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("type"), TEXT("remove"));

	const TSharedPtr<FJsonObject> ItemJson = MakeShareable(new FJsonObject);
	ItemJson->SetNumberField(TEXT("id"), Id);
	JsonObject->SetObjectField(TEXT("item"), ItemJson);
	

	FString OutputString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	Socket->Send(OutputString);
}

void UChecklistApiComponent::ToggleItem(const int Id, const bool bNewState)
{
	const TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("type"), bNewState ? TEXT("check") : TEXT("uncheck"));

	const TSharedPtr<FJsonObject> ItemJson = MakeShareable(new FJsonObject);
	ItemJson->SetNumberField(TEXT("id"), Id);
	JsonObject->SetObjectField(TEXT("item"), ItemJson);
	

	FString OutputString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	Socket->Send(OutputString);
}

