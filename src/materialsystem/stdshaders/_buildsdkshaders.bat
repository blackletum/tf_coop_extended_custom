@echo off
setlocal

rem == Setup path to nmake.exe, from vc 2013 common tools directory ==
call "%VS120COMNTOOLS%vsvars32.bat"

set TTEXE=..\..\devtools\bin\timeprecise.exe
if not exist %TTEXE% goto no_ttexe
goto no_ttexe_end

:no_ttexe
set TTEXE=time /t
:no_ttexe_end

rem echo.
rem echo ~~~~~~ buildsdkshaders %* ~~~~~~
%TTEXE% -cur-Q
set tt_all_start=%ERRORLEVEL%
set tt_all_chkpt=%tt_start%

set BUILD_SHADER=call _buildshaders.bat
set ARG_EXTRA=

REM %BUILD_SHADER% _stdshader_dx9_20b		-game %GAMEDIR% -source %SOURCEDIR%
REM %BUILD_SHADER% _stdshader_dx9_30			-game %GAMEDIR% -source %SOURCEDIR% -dx9_30	-force30 
%BUILD_SHADER% mymod_dx9_20b			-game %GAMEDIR% -source %SOURCEDIR%
%BUILD_SHADER% mymod_dx9_30				-game %GAMEDIR% -source %SOURCEDIR% -dx9_30	-force30