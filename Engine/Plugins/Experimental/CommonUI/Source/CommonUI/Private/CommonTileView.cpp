// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonTileView.h"
#include "CommonUIPrivatePCH.h"
#include "SCommonButtonTableRow.h"

///////////////////////
// SCommonTileView
///////////////////////

template <typename ItemType>
class SCommonTileView : public STileView<ItemType>
{
public:
	FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
	{
		if (bScrollToSelectedOnFocus && (InFocusEvent.GetCause() == EFocusCause::Navigation || InFocusEvent.GetCause() == EFocusCause::SetDirectly))
		{
			if (this->ItemsSource && this->ItemsSource->Num() > 0)
			{
				if(this->GetNumItemsSelected() == 0)
				{
					ItemType FirstItem = (*this->ItemsSource)[0];
					this->SetSelection(FirstItem, ESelectInfo::OnNavigation);
					this->RequestNavigateToItem(FirstItem, InFocusEvent.GetUser());
				}
				else
				{
					TArray<ItemType> ItemArray;
					this->GetSelectedItems(ItemArray);

					ItemType FirstSelected = ItemArray[0];
					this->SetSelection(FirstSelected, ESelectInfo::OnNavigation);
					this->RequestNavigateToItem(FirstSelected, InFocusEvent.GetUser());
				}
			}
		}
		bScrollToSelectedOnFocus = true;
		return STileView<ItemType>::OnFocusReceived(MyGeometry, InFocusEvent);
	}

	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override
	{
		STileView<ItemType>::OnMouseLeave(MouseEvent);
	}

private:
	bool bScrollToSelectedOnFocus = true;
};


///////////////////////
// UCommonTileView
///////////////////////

UCommonTileView::UCommonTileView(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bEnableScrollAnimation = true;
}

TSharedRef<STableViewBase> UCommonTileView::RebuildListWidget()
{
	return ConstructTileView<SCommonTileView>();
}

UUserWidget& UCommonTileView::OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable)
{
	if (DesiredEntryClass->IsChildOf<UCommonButtonBase>())
	{
		return GenerateTypedEntry<UUserWidget, SCommonButtonTableRow<UObject*>>(DesiredEntryClass, OwnerTable);
	}
	return GenerateTypedEntry(DesiredEntryClass, OwnerTable);
}