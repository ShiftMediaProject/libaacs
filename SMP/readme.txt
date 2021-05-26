
This is a small list of steps in order to build libbz2 into a msvc dll and/or lib file.

The project contains Release and Debug builds for static lib files (Debug/Release)
  as well as dynamic shared dll files (DebugDLL/ReleaseDLL). Along with the standard
  windows dll/lib configurations mentioned above there are also equivalent variants that
  can be used to compile for WinRT/UWP (These configurations have a WinRT suffix).
There are also architecture configurations for either 32bit (x86) or 64bit (x64) compilation.
Choose whichever project configuration meets your requirements.

The project configurations support being built with various different windows SDK versions.
  By default they will use the lowest SDK version that would be available for Visual Studio
  version 2013 and up (This is the 8.1 SDK). However a batch file is also included 
  (libaacs_with_latest_sdk.bat) which can be used to auto detect the newest available SDK 
  installed on the host machine and then open the project using that as the compilation SDK.
  
When using the WinRT/UWP project configurations the projects will automatically compile towards
  the default application target for the Version of Visual Studio being used:
  VS 2013: 8.1
  VS 2015: 8.1
  VS 2017+: 10.0.10240.0

*** Generating keydbcfg files ***

The build system requires several of the keydbcfg-*.c files to be built automatically. This should have already been done and the resulting
source files will be found in the SMC\src directory. If these files are missing then the project will try and automatically
generate new ones. If the files need to be forced to update then they can be simply deleted which will result in new files
being generated the next time the project is built.

In order for new files to be generated the project needs access to 'Flex' and 'Bison'. Ensure that these are accessible by Visual Studio.

Both can be downloaded from here:
http://sourceforge.net/projects/winflexbison

Pre-built windows binaries can also be found in the above link. To enable generation of new files 'win_flex' and 'win_bison' should be extracted 
and added somewhere accessible to the Visual Studio path (e.g. extract it into the Visual Studio) project folder.