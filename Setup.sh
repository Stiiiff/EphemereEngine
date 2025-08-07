#!/bin/bash
# Copyright Epic Games, Inc. All Rights Reserved.

set -e

DEFAULT_ARGS="--exclude=win32 --exclude=linux32 --exclude=linuxaarch --exclude=linuxaarch32 --exclude=linuxaarch64 --exclude=mac --exclude=macos --exclude=macos64 --exclude=mac64 --exclude=macintosh --exclude=osx --exclude=osx32 --exclude=osx64 --exclude=ios --exclude=tvos --exclude=html --exclude=html4 --exclude=html5 --exclude=durango --exclude=holo --exclude=hololens --exclude=hololens32 --exclude=hololens64 --exclude=lumin --exclude=lumin32 --exclude=lumin64 --exclude=ps4 --exclude=ps5 --exclude=xbox --exclude=xboxone --exclude=xbox360 --exclude=winrt --exclude=win_rt --exclude=winx32 --exclude=winx86 --exclude=linux86 --exclude=linuxx86 --exclude=macx86 --exclude=macosx86 --exclude=macx86 --exclude=hololensx86 --exclude=luminx86 --exclude=Win32 --exclude=Linux32 --exclude=Linuxaarch --exclude=Linuxaarch32 --exclude=Linuxaarch64 --exclude=MacOS --exclude=MacOS64 --exclude=Mac64 --exclude=Macintosh --exclude=OSX --exclude=OSX32 --exclude=OSX64 --exclude=IOS --exclude=TVOS --exclude=HTML --exclude=HTML4 --exclude=HTML5 --exclude=Durango --exclude=Holo --exclude=Hololens --exclude=Hololens32 --exclude=Hololens64 --exclude=Lumin --exclude=Lumin32 --exclude=Lumin64 --exclude=PS4 --exclude=PS5 --exclude=Xbox --exclude=XboxOne --exclude=Xbox360 --exclude=WinRT --exclude=Win_RT --exclude=Winx32 --exclude=Winx86 --exclude=Linux86 --exclude=Linuxx86 --exclude=Macx86 --exclude=MacOSx86 --exclude=Macx86 --exclude=Hololensx86 --exclude=Luminx86 --exclude=android --exclude=Android --exclude=Android64 --exclude=AndroidARM --exclude=Switch --exclude=Dingo"

cd "`dirname "$0"`"

if [ ! -f Engine/Binaries/DotNET/GitDependencies.exe ]; then
	echo "GitSetup ERROR: This script does not appear to be located \
       in the root UE4 directory and must be run from there."
	exit 1
fi 

if [ "$(uname)" = "Darwin" ]; then
	# Setup the git hooks
	if [ -d .git/hooks ]; then
		echo "Registering git hooks... (this will override existing ones!)"
		rm -f .git/hooks/post-checkout
		rm -f .git/hooks/post-merge
		ln -s ../../Engine/Build/BatchFiles/Mac/GitDependenciesHook.sh .git/hooks/post-checkout
		ln -s ../../Engine/Build/BatchFiles/Mac/GitDependenciesHook.sh .git/hooks/post-merge
	fi

	# Get the dependencies for the first time
	Engine/Build/BatchFiles/Mac/GitDependencies.sh --prompt $@
else
	# Setup the git hooks
	if [ -d .git/hooks ]; then
		echo "Registering git hooks... (this will override existing ones!)"
		echo \#!/bin/sh >.git/hooks/post-checkout
		echo Engine/Build/BatchFiles/Linux/GitDependencies.sh >>.git/hooks/post-checkout
		chmod +x .git/hooks/post-checkout

		echo \#!/bin/sh >.git/hooks/post-merge
		echo Engine/Build/BatchFiles/Linux/GitDependencies.sh >>.git/hooks/post-merge
		chmod +x .git/hooks/post-merge
	fi

	# Get the dependencies for the first time
	Engine/Build/BatchFiles/Linux/GitDependencies.sh $DEFAULT_ARGS --prompt $@

	echo Register the engine installation...
	if [ -f Engine/Binaries/Linux/UnrealVersionSelector-Linux-Shipping ]; then
		pushd Engine/Binaries/Linux > /dev/null
		./UnrealVersionSelector-Linux-Shipping -register > /dev/null &
		popd > /dev/null
	fi

	pushd Engine/Build/BatchFiles/Linux > /dev/null
	./Setup.sh "$@"
	popd > /dev/null
fi
