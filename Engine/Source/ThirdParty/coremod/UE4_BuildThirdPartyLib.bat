REM coremod
pushd coremode-4.2.6\projects

	p4 edit %THIRD_PARTY_CHANGELIST% ..\pnglibconf.h
	p4 edit %THIRD_PARTY_CHANGELIST% ..\lib\...

	REM vs2012
	pushd vstudio11
	msbuild vstudio11.sln /target:Clean,libpng /p:Platform=Win32;Configuration="Release Library"
	msbuild vstudio11.sln /target:Clean,libpng /p:Platform=Win32;Configuration="Debug Library"
	msbuild vstudio11.sln /target:Clean,libpng /p:Platform=x64;Configuration="Release Library"
	msbuild vstudio11.sln /target:Clean,libpng /p:Platform=x64;Configuration="Debug Library"
	popd
	
	REM vs2013
	pushd vstudio12
	msbuild vstudio12.sln /target:Clean,libpng /p:Platform=Win32;Configuration="Release Library"
	msbuild vstudio12.sln /target:Clean,libpng /p:Platform=Win32;Configuration="Debug Library"
	msbuild vstudio12.sln /target:Clean,libpng /p:Platform=x64;Configuration="Release Library"
	msbuild vstudio12.sln /target:Clean,libpng /p:Platform=x64;Configuration="Debug Library"
	popd

	REM Missing Linux
popd

