// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDInventoryComponent.generated.h"

UENUM(BlueprintType)
enum class ERANDItem : uint8
{
	None			UMETA(DisplayName = "None"),
	Sidearm			UMETA(DisplayName = "Sidearm"),
	BurnerPhone		UMETA(DisplayName = "Burner phone"),
	TenderDossier	UMETA(DisplayName = "Tender dossier"),
	FakeInvoice		UMETA(DisplayName = "Fake invoice"),
	WarehouseKey	UMETA(DisplayName = "City Deep key"),
};

UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDInventoryComponent();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(ERANDItem Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(ERANDItem Item);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(ERANDItem Item) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	const TArray<ERANDItem>& GetItems() const { return Items; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FString GetItemName(ERANDItem Item) const;

	void LoadItems(const TArray<ERANDItem>& InItems) { Items = InItems; }

private:
	UPROPERTY()
	TArray<ERANDItem> Items;
};
