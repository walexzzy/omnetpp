@echo off
echo.
echo Press ENTER to convert all INI files in the current directory:
cd
echo.
pause
dir /s /b *.ini >inifiles.lst
perl %~dp0\migrateini.pl inifiles.lst
echo REMINDER1: INI files now have named configuration. Revise the names of the configuration.
echo.
echo REMINDER2: Configurations can extend each other. Common parts between different configuration can be moved to a common ancestor.
echo.
echo REMINDER3: NED file parameters now support default values. Specifying **.apply-default=true at the end of the configuration allows to fetch the unspecified parameter values from the NED file.