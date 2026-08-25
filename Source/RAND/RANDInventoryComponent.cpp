// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDInventoryComponent.h"

URANDInventoryComponent::URANDInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URANDInventoryComponent::AddItem(ERANDItem Item)
{
	if (Item == ERANDItem::None || HasItem(Item)) return;
	Items.Add(Item);
}

bool URANDInventoryComponent::RemoveItem(ERANDItem Item)
{
	return Items.Remove(Item) > 0;
}

bool URANDInventoryComponent::HasItem(ERANDItem Item) const
{
	return Items.Contains(Item);
}

FString URANDInventoryComponent::GetItemName(ERANDItem Item) const
{
	switch (Item)
	{
	case ERANDItem::Sidearm:        return TEXT("Sidearm");
	case ERANDItem::BurnerPhone:    return TEXT("Burner phone");
	case ERANDItem::TenderDossier:  return TEXT("Tender dossier");
	case ERANDItem::FakeInvoice:    return TEXT("Fake invoice book");
	case ERANDItem::WarehouseKey:   return TEXT("City Deep key");
	default:                        return TEXT("Nothing");
	}
}
