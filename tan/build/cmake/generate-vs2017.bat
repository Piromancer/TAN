SETLOCAL

RMDIR /S /Q vs2017
MKDIR vs2017
CD vs2017

SET CMAKE_PREFIX_PATH=C:\Qt\Qt5.6.0\5.6\msvc2015_64\lib\cmake\Qt5Widgets
cmake .. -G "Visual Studio 15 2017" -A x64 -DOpenCL_INCLUDE_DIR=..\..\..\..\thirdparty\OpenCL-Headers -DPortAudio_DIR=..\..\..\..\..\thirdparty\portaudio -DIPP_DIR="C:\Program Files (x86)\IntelSWTools\compilers_and_libraries_2020.0.166\windows\ipp" -DDEFINE_AMD_OPENCL_EXTENSION=1
