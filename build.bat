@call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

@echo off

pushd build
@for %%i in (*.*) do if not "%%i"=="SDL2.dll" if not "%%i"=="clear.bat"  if not "%%i"=="baran_logo.bmp"  if not "%%i"=="libfreetype-6.dll"  if not "%%i"=="SDL2_ttf.dll" if not "%%i"=="zlib1.dll" del /q "%%i"
@cl -Zi /Febuild.exe %~dp0source\main.c /I %~dp0include /link /LIBPATH:%~dp0lib SDL2_ttf.lib SDL2.lib SDL2main.lib Shell32.lib /SUBSYSTEM:CONSOLE
start "" build.exe
popd

