@echo off
setlocal
pushd "%~dp0"

rem Figure out if we should append the -prompt argument
set PROMPT_ARGUMENT=
set DEFAULT_ARGS=--exclude=win32 --exclude=linux32 --exclude=linuxaarch --exclude=linuxaarch32 --exclude=linuxaarch64 --exclude=mac --exclude=macos --exclude=macos64 --exclude=mac64 --exclude=macintosh --exclude=osx --exclude=osx32 --exclude=osx64 --exclude=ios --exclude=tvos --exclude=html --exclude=html4 --exclude=html5 --exclude=durango --exclude=holo --exclude=hololens --exclude=hololens32 --exclude=hololens64 --exclude=lumin --exclude=lumin32 --exclude=lumin64 --exclude=ps4 --exclude=ps5 --exclude=xbox --exclude=xboxone --exclude=xbox360 --exclude=winrt --exclude=win_rt --exclude=winx32 --exclude=winx86 --exclude=linux86 --exclude=linuxx86 --exclude=macx86 --exclude=macosx86 --exclude=macx86 --exclude=hololensx86 --exclude=luminx86 --exclude=Win32 --exclude=Linux32 --exclude=Linuxaarch --exclude=Linuxaarch32 --exclude=Linuxaarch64 --exclude=MacOS --exclude=MacOS64 --exclude=Mac64 --exclude=Macintosh --exclude=OSX --exclude=OSX32 --exclude=OSX64 --exclude=IOS --exclude=TVOS --exclude=HTML --exclude=HTML4 --exclude=HTML5 --exclude=Durango --exclude=Holo --exclude=Hololens --exclude=Hololens32 --exclude=Hololens64 --exclude=Lumin --exclude=Lumin32 --exclude=Lumin64 --exclude=PS4 --exclude=PS5 --exclude=Xbox --exclude=XboxOne --exclude=Xbox360 --exclude=WinRT --exclude=Win_RT --exclude=Winx32 --exclude=Winx86 --exclude=Linux86 --exclude=Linuxx86 --exclude=Macx86 --exclude=MacOSx86 --exclude=Macx86 --exclude=Hololensx86 --exclude=Luminx86 --exclude=android --exclude=Android --exclude=Android64 --exclude=AndroidARM --exclude=Switch --exclude=Dingo
for %%P in (%*) do if /I "%%P" == "--prompt" goto no_prompt_argument
for %%P in (%*) do if /I "%%P" == "--force" goto no_prompt_argument
set PROMPT_ARGUMENT=--prompt
:no_prompt_argument

rem Sync the dependencies...
.\Engine\Binaries\DotNET\GitDependencies.exe %DEFAULT_ARGS% %PROMPT_ARGUMENT% %*
if ERRORLEVEL 1 goto error

rem Setup the git hooks...
if not exist .git\hooks goto no_git_hooks_directory
echo Registering git hooks...
echo #!/bin/sh >.git\hooks\post-checkout
echo Engine/Binaries/DotNET/GitDependencies.exe %* >>.git\hooks\post-checkout
echo #!/bin/sh >.git\hooks\post-merge
echo Engine/Binaries/DotNET/GitDependencies.exe %* >>.git\hooks\post-merge
:no_git_hooks_directory

rem Install prerequisites...
echo Installing prerequisites...
start /wait Engine\Extras\Redist\en-us\UE4PrereqSetup_x64.exe /quiet

rem Register the engine installation...
if not exist .\Engine\Binaries\Win64\UnrealVersionSelector-Win64-Shipping.exe goto :no_unreal_version_selector
.\Engine\Binaries\Win64\UnrealVersionSelector-Win64-Shipping.exe /register
:no_unreal_version_selector

rem Done!
goto :end

rem Error happened. Wait for a keypress before quitting.
:error
pause

:end
popd
