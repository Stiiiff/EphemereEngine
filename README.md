EphemereEngine 
=============
Built on top of 4.27.2-chaos branch with some of the changes from Noeklair on Ornis branch. 

Differences between the official build and the EphemereEngine fork :

Added:
- New GBuffer layout  to allow for new ObjectNormal Buffer
- Shading Models redisign for the specific use of Darkbloom Visuals artstyle
- Custom Normal Packing Encoding
- Subpixel Morphological Anti-Aliasing (SMAA)
- More to come ..

Removed:
- Non-common plugins for game development wiped out
- Libraries from unused platforms are excluded from the setup script
- Non-desktop platforms, including Mac
- Unused third-party libraries are removed
- Most of the VR stuff is removed
- Feature packs
- Samples
- Starter content
- Templates
- Datasmith
- OpenGL
- DirectX 9/10/11
- Virtual production
- PhysX3, Apex, and Blast
- Prereq installer

Improved:
- Vulkan version is updated to 1.3.296.0
- Vulkan is the default RHI for this branch
- Some of the assets are either cleaned up or fixed, so they are not getting downloaded by Epic Games' servers and this repo is going to be used instead
- Many different fixes are merged from the up-to-date branch
- Some of the default settings are increased in quality, or changed for quality of life improvements


Unreal Engine
=============

Please refer to the original [README](https://github.com/EpicGames/UnrealEngine/blob/release/README.md) file from the Epic Games for common info.

Licensing and Contributions
---------------------------

Your access to and use of Unreal Engine on GitHub is governed by the [Unreal Engine End User License Agreement](https://www.unrealengine.com/eula). If you don't agree to those terms, as amended from time to time, you are not permitted to access or use Unreal Engine.

We welcome any contributions to Unreal Engine development through [pull requests](https://github.com/EpicGames/UnrealEngine/pulls/) on GitHub. Most of our active development is in the **master** branch, so we prefer to take pull requests there (particularly for new features). We try to make sure that all new code adheres to the [Epic coding standards](https://docs.unrealengine.com/latest/INT/Programming/Development/CodingStandard/).  All contributions are governed by the terms of the EULA.
