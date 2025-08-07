// Copyright Epic Games, Inc. All Rights Reserved.

#include "StandaloneRenderer.h"
#include "Misc/CommandLine.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Styling/CoreStyle.h"
#include "StandaloneRendererLog.h"

#include "Rendering/SlateRenderer.h"

DEFINE_LOG_CATEGORY(LogStandaloneRenderer);

/**
 * Single function to create the standalone renderer for the running platform
 */
TSharedRef<FSlateRenderer> GetStandardStandaloneRenderer()
{
	// create a standalone renderer object
	TSharedPtr<FSlateRenderer> Renderer = NULL;

	// enforce non-NULL pointer
	return Renderer.ToSharedRef();
}

class FStandaloneRenderer : public IModuleInterface
{
};

IMPLEMENT_MODULE( FStandaloneRenderer, StandaloneRenderer )
