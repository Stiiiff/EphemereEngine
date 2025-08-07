// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RendererInterface.h"

class FViewInfo;
class FLightSceneInfo;
class FLightSceneProxy;
class FSceneViewFamily;

// Returns whether light shafts globally are enabled.
extern bool ShouldRenderLightShafts(const FSceneViewFamily& ViewFamily);

// Returns whether the light proxy is eligible for light shaft rendering. Assumes light shafts are enabled.
extern bool ShouldRenderLightShaftsForLight(const FViewInfo& View, const FLightSceneProxy& LightSceneProxy);