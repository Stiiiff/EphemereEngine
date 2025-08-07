REM libPNG
pushd libPNG-1.5.2\projects

	p4 edit %THIRD_PARTY_CHANGELIST% ..\pnglibconf.h
	p4 edit %THIRD_PARTY_CHANGELIST% ..\lib\...

	REM vs2015
	pushd vstudio14
	msbuild vstudio14.sln /target:Clean,libpng /p:Platform=Win32;Configuration="Release Library"
	msbuild vstudio14.sln /target:Clean,libpng /p:Platform=Win32;Configuration="Debug Library"
	msbuild vstudio14.sln /target:Clean,libpng /p:Platform=x64;Configuration="Release Library"
	msbuild vstudio14.sln /target:Clean,libpng /p:Platform=x64;Configuration="Debug Library"
	popd
	
	REM Missing Linux
popd

