// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"


class ISerializedRecorderInterfaceModule
	: public IModuleInterface
{
public:

	static ISerializedRecorderInterfaceModule& Get()
	{
        return FModuleManager::LoadModuleChecked<ISerializedRecorderInterfaceModule>("Interface");
	}

public:

	/** Virtual destructor. */
	virtual ~ISerializedRecorderInterfaceModule() { }
};

