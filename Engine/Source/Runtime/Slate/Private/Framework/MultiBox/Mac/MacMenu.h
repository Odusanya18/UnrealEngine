// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MultiBox.h"
#include "SMenuEntryBlock.h"

@interface FMacMenu : NSMenu <NSMenuDelegate>
@property (assign) TSharedPtr<const FMenuEntryBlock> MenuEntryBlock;
@property (assign) TSharedPtr<const FMultiBox> MultiBox;
@end

class SLATE_API FSlateMacMenu
{
public:

	static void UpdateWithMultiBox(const TSharedRef<FMultiBox>& MultiBox);
	static void UpdateMenu(FMacMenu* Menu);
	static void ExecuteMenuItemAction(const TSharedRef<const FMenuEntryBlock>& Block);

private:

	static NSString* GetMenuItemTitle(const TSharedRef< const FMenuEntryBlock >& Block);
	static NSImage* GetMenuItemIcon(const TSharedRef<const FMenuEntryBlock>& Block);
	static NSString* GetMenuItemKeyEquivalent(const TSharedRef<const FMenuEntryBlock>& Block, uint32* OutModifiers);
	static bool IsMenuItemEnabled(const TSharedRef<const FMenuEntryBlock>& Block);
	static int32 GetMenuItemState(const TSharedRef<const FMenuEntryBlock>& Block);
};
