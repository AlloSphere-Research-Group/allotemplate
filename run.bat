@echo off

cmake --build build/Release --config Release -j %NUMBER_OF_PROCESSORS% -- /p:CL_MPCount=3
@REM cmake --build build/Debug --config Debug -j %NUMBER_OF_PROCESSORS% -- /p:CL_MPCount=3

if %ERRORLEVEL% EQU 0 cd bin & .\app_name_here