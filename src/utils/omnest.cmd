@echo off
SET IDEDIR=%~dp0\..\ide
SET LAUNCHER=omnest
SET PATH=%~dp0;%~dp0\..\msys\bin;%~dp0\..\mingw\bin;%PATH%
echo Starting the OMNEST IDE...
start %IDEDIR%\%LAUNCHER% %* >out.log
