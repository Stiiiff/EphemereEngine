// Copyright Epic Games, Inc. All Rights Reserved.


#include "UnrealMultiUserServerRun.h"
#include "RequiredProgramMainCPPInclude.h"

IMPLEMENT_APPLICATION(UnrealMultiUserServer, "UnrealMultiUserServer");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	return RunUnrealMultiUserServer(ArgC, ArgV);
}
